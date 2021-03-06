#ifndef FRAMEWORK_H
#define FRAMEWORK_H

#include "engine.h"
#include <queue>
#include <memory>
#include <unordered_map>
#include <stack>
#include <bitset>

namespace Timing {
    typedef void (*timer_complete_func)();
    
    struct Timer {
        float elapsed = 0.0f;
        float time = 0.0f;
        timer_complete_func on_elapsed = nullptr; 
    };

    static std::vector<Timer> _timers;

    inline void init(int sz) {
        _timers.reserve(sz);
    }
    
    inline void add_timer(float time, timer_complete_func on_elapsed) {
        _timers.push_back({ 0.0f, time, on_elapsed });
    }
    
    inline void update_timers() {
        for(size_t i = 0; i < _timers.size(); i++) {
            _timers[i].elapsed += Time::delta_time;
            if(_timers[i].elapsed > _timers[i].time) {
                _timers[i].on_elapsed();
            }
        }
        
        for(size_t i = 0; i < _timers.size(); i++) {
            if(_timers[i].elapsed > _timers[i].time) {
                _timers[i] = _timers[_timers.size() - 1];
                _timers.erase(_timers.end() - 1);
            }
        }
    }
};

namespace ECS {
    const unsigned ENTITY_INDEX_BITS = 22;
    const unsigned ENTITY_INDEX_MASK = (1<<ENTITY_INDEX_BITS)-1;

    const unsigned ENTITY_GENERATION_BITS = 8;
    const unsigned ENTITY_GENERATION_MASK = (1<<ENTITY_GENERATION_BITS)-1;

    typedef unsigned EntityId;
    struct Entity {
        EntityId id = 0;

        unsigned index() const { return id & ENTITY_INDEX_MASK; }
        unsigned generation() const { return (id >> ENTITY_INDEX_BITS) & ENTITY_GENERATION_MASK; }
    };

    class ComponentID {
        static size_t counter;
        public:
            template<typename T>
            static size_t value() {
                static size_t id = counter++;
                return id;
            }
    };

    const unsigned MINIMUM_FREE_INDICES = 1024;

    struct EntityManager {
        std::vector<unsigned char> _generation;
        std::queue<unsigned> _free_indices;

        Entity create() {
            unsigned idx;
            if (_free_indices.size() > MINIMUM_FREE_INDICES) {
                idx = _free_indices.front();
                _free_indices.pop();
            } else {
                _generation.push_back(0);
                idx = _generation.size() - 1;
                ASSERT_WITH_MSG(idx < (1 << ENTITY_INDEX_BITS), "idx is malformed, larger than 22 bits?");
            }

            return make_entity(idx, _generation[idx]);
        }

        Entity make_entity(unsigned idx, unsigned char generation) {
            Entity e;
            auto id = generation << ENTITY_INDEX_BITS | idx;
            e.id = id;
            return e;
        }

        bool alive(Entity e) const {
            return _generation[e.index()] == e.generation();
        }

        void destroy(Entity e) {
            if(!alive(e))
                return;

            const unsigned idx = e.index();
            ++_generation[idx];
            _free_indices.push(idx);
        }
    };

    struct EntityData {    
        struct BaseContainer {
            virtual void move(int index, int last_index) = 0;
        };

        template<typename T>
        struct ComponentContainer : BaseContainer {
            std::vector<T> *items;

            void move(int index, int last_index) override {
                items->at(index) = items->at(last_index);
            }
        };
        
        std::vector<Entity> entity;
        std::unordered_map<EntityId, unsigned> _map;
        std::vector<BaseContainer*> containers;
        
        size_t size = 0;
        int length = 0;
        void allocate_entities(size_t sz) {
            size = sz;
            entity.reserve(size);
        }

        template<typename T>
        void initialize(std::vector<T> *items) {
            items->reserve(size);
            for(size_t i = 0; i < size; ++i) {
                items->emplace_back();
            }
            auto c = new ComponentContainer<T>();
            c->items = items;
            containers.push_back(c);
        }

        static const int invalid_handle = -1;

        struct Handle {
            int i = -1;
        };
        
        void add_entity(Entity e) {
            ASSERT_WITH_MSG(entity.size() <= size, "Component storage is full, n:" + std::to_string(entity.size()));
            ASSERT_WITH_MSG(!contains(e), "Entity already has component");
            
            unsigned int index = entity.size();
            _map[e.id] = index;
            entity.push_back(e);

            ++length;
        }

        Handle get_handle(Entity e) {
            auto a = _map.find(e.id);
            if(a != _map.end()) {
                return { (int)a->second };
            }
            return { invalid_handle };
        }

        const Handle get_handle(Entity e) const {
            auto a = _map.find(e.id);
            if(a != _map.end()) {
                return { (int)a->second };
            }
            return { invalid_handle };
        }

        bool contains(Entity e) {
            auto a = _map.find(e.id);
            return a != _map.end();
        }

        const bool contains(Entity e) const {
            auto a = _map.find(e.id);
            return a != _map.end();
        }

        bool is_valid(Handle h) {
            return h.i != invalid_handle;
        }

        void clear_all_entities() {
            for(int i = length - 1; i >= 0; i--) {
                remove(entity[i]);
            }
        }

        void remove(Entity e) {
            if(!contains(e))
                return;

            auto a = _map.find(e.id);
            const int index = a->second;
            const int lastIndex = entity.size() - 1;

            if (lastIndex >= 0) {
                // Get the entity at the index to destroy
                Entity entityToDestroy = entity[index];
                // Get the entity at the end of the array
                Entity lastEntity = entity[lastIndex];

                // Move last entity's data
                entity[index] = entity[lastIndex];

                for(size_t i = 0; i < containers.size(); i++) {
                    containers[i]->move(index, lastIndex);
                }

                // Update map entry for the swapped entity
                _map[lastEntity.id] = index;
                // Remove the map entry for the destroyed entity
                _map.erase(entityToDestroy.id);

                // Decrease count
                entity.pop_back();

                --length;
            }
        }
    };

    typedef std::bitset<512> ComponentMask;
        
    struct BaseContainer {
        virtual void move(int index, int last_index) = 0;
    };

    template<typename T>
    struct ComponentContainer : BaseContainer {
        std::vector<T> items;

        void move(int index, int last_index) override {
            items.at(index) = items.at(last_index);
        }
    };

    class EntityDataDynamic {
        public:
            ComponentMask mask;
            int length = 0;
            std::vector<Entity> entity;
            struct Handle {
                int i = -1;
            };

            template <typename ... Components>
            void allocate_entities(size_t sz) {
                size = sz;

                size_t max_components_total = 512;

                containers.reserve(max_components_total);
                for(size_t i = 0; i < max_components_total; ++i) {
                    containers.emplace_back();
                }
                has_component.reserve(max_components_total);
                for(size_t i = 0; i < max_components_total; ++i) {
                    has_component.emplace_back(false);
                }
                entity.reserve(size);
                init<Components...>(sz);
            }
        
            void add_entity(Entity e) {
                ASSERT_WITH_MSG(entity.size() <= size, "Component storage is full, n:" + std::to_string(entity.size()));
                ASSERT_WITH_MSG(!contains(e), "Entity already has component");
                
                unsigned int index = entity.size();
                _map[e.id] = index;
                entity.push_back(e);

                ++length;
            }

            void remove(Entity e) {
                if(!contains(e))
                    return;

                auto a = _map.find(e.id);
                const int index = a->second;
                const int lastIndex = entity.size() - 1;

                if (lastIndex >= 0) {
                    // Get the entity at the index to destroy
                    Entity entityToDestroy = entity[index];
                    // Get the entity at the end of the array
                    Entity lastEntity = entity[lastIndex];

                    // Move last entity's data
                    entity[index] = entity[lastIndex];

                    for(size_t &i : container_indexes) {
                        containers[i]->move(index, lastIndex);
                    }

                    /*
                    for(size_t i = 0; i < container_indexes.size(); i++) {
                        containers[i]->move(index, lastIndex);
                    }
                    */

                    // Update map entry for the swapped entity
                    _map[lastEntity.id] = index;
                    // Remove the map entry for the destroyed entity
                    _map.erase(entityToDestroy.id);

                    // Decrease count
                    entity.pop_back();

                    --length;
                }
            }

            Handle get_handle(Entity e) {
                auto a = _map.find(e.id);
                if(a != _map.end()) {
                    return { (int)a->second };
                }
                return { invalid_handle };
            }

            const Handle get_handle(Entity e) const {
                auto a = _map.find(e.id);
                if(a != _map.end()) {
                    return { (int)a->second };
                }
                return { invalid_handle };
            }

            bool is_valid_handle(Handle h) {
                return h.i != invalid_handle;
            }

            template <typename C>
            C &get(const Handle &handle) {
                auto container_index = ComponentID::value<C>();
                return static_cast<ComponentContainer<C>*>(containers[container_index])->items[handle.i];
            }

            template <typename C>
            void set(const Handle &handle, const C &component) {
                auto container_index = ComponentID::value<C>();
                static_cast<ComponentContainer<C>*>(containers[container_index])->items[handle.i] = component;
            }

            template <typename C>
            C &index(const int index) {
                auto container_index = ComponentID::value<C>();
                return static_cast<ComponentContainer<C>*>(containers[container_index])->items[index];
            }

            template <typename C>
            std::vector<C> &get_components_by_type() {
                auto container_index = ComponentID::value<C>();
                return static_cast<ComponentContainer<C>*>(containers[container_index])->items;
            }

            template <typename C>
            bool match() {
                auto container_index = ComponentID::value<C>();
                return has_component[container_index];
            }

            template <typename C1, typename C2, typename ... Components>
            bool match() {
                return match<C1>() && match<C2, Components ...>();
            }

        private:
            static const int invalid_handle = -1;
            std::unordered_map<EntityId, unsigned> _map;
            std::vector<size_t> container_indexes;
            std::vector<bool> has_component;
            std::vector<BaseContainer*> containers;
            size_t size = 0;

            template <typename C>
            void init(size_t sz) {
                auto c = new ComponentContainer<C>();
                // This only works if you want to have components without constructors
                c->items.reserve(sz);
                for(size_t i = 0; i < sz; ++i) {
                    c->items.emplace_back();
                }
                auto container_index = ComponentID::value<C>();
                container_indexes.push_back(container_index);
                has_component[container_index] = true;
                containers[container_index] = c;
            }

            template <typename C1, typename C2, typename ... Components>
            void init(size_t sz) {
                init<C1>(sz);
                init<C2, Components ...>(sz);
            }
            
            bool contains(Entity e) {
                auto a = _map.find(e.id);
                return a != _map.end();
            }
    };

    struct ArcheType {
        ComponentMask _mask;
    };

    template <typename C>
    ComponentMask create_mask() {
        ComponentMask mask;
        mask.set(ComponentID::value<C>());
        return mask;
    }

    template <typename C1, typename C2, typename ... Components>
    ComponentMask create_mask() {
        return create_mask<C1>() | create_mask<C2, Components ...>();
    }

    struct ArchetypeManager {
        private:
        EntityManager em;
        std::vector<EntityDataDynamic*> archetypes;
        std::unordered_map<ComponentMask, int> archetype_map;
        std::unordered_map<EntityId, ComponentMask> entity_to_archetype;

        public:
        template <typename ... Components>
        ArcheType create_archetype(size_t sz) {
            EntityDataDynamic *container = new EntityDataDynamic();
            container->allocate_entities<Components...>(sz);
            archetypes.push_back(container);
            ArcheType a;
            a._mask = create_mask<Components...>();
            archetype_map[a._mask] = archetypes.size() - 1;
            container->mask = a._mask;
            return a;
        }

        struct ContainerIterator {
            std::vector<EntityDataDynamic*> containers;
            // std::vector<ArcheType> archetypes;
        };

        template <typename ... Components>
        ContainerIterator get_iterator() {
            ContainerIterator it;
            for(auto c : archetypes) {
                if(c->match<Components...>()) {
                    it.containers.push_back(c);
                    // ArcheType a;
                    // a._mask = c->mask;
                    // it.archetypes.push_back(a);
                }
            }
            return it;
        }

        Entity create_entity(const ArcheType &a) {
            auto entity = em.create();
            archetypes[archetype_map[a._mask]]->add_entity(entity);
            entity_to_archetype[entity.id] = a._mask;
            return entity;
        }

        void remove_entity(const ArcheType &a, Entity entity) {
            archetypes[archetype_map[a._mask]]->remove(entity);
            entity_to_archetype.erase(entity.id);
        }

        bool is_alive(const ArcheType &a, Entity entity) {
            EntityDataDynamic *data = archetypes[archetype_map[a._mask]];
            auto handle = data->get_handle(entity);
            return data->is_valid_handle(handle);
        }

        ArcheType get_archetype(Entity entity) {
            ASSERT_WITH_MSG(entity_to_archetype.find(entity.id) != entity_to_archetype.end(), "Entity is not in any archetype");
            return { entity_to_archetype[entity.id] };
        }

        template<typename T>
        void set_component(const ArcheType &a, Entity entity, const T &component) {
            EntityDataDynamic *data = archetypes[archetype_map[a._mask]];
            auto handle = data->get_handle(entity);
            ASSERT_WITH_MSG(data->is_valid_handle(handle), "set_component: Invalid entity handle! Check if entity is alive first!?");
            data->set(handle, component);
        }

        template<typename T>
        T &get_component(const ArcheType &a, Entity entity) {
            EntityDataDynamic *data = archetypes[archetype_map[a._mask]];
            auto handle = data->get_handle(entity);
            ASSERT_WITH_MSG(data->is_valid_handle(handle), "get_component: Invalid entity handle! Check if entity is alive first!?");
            return data->get<T>(handle);
        }
    };
};

namespace Intersects {
    inline bool circle_contains_point(Vector2 circle, float radius, Vector2 point) {
        float circle_left = circle.x - radius;
        float circle_right = circle.x + radius;
        float circle_bottom = circle.y + radius;
        float circle_top = circle.y - radius;
        //  Check if point is inside bounds
        if (radius > 0 && point.x >= circle_left && point.x <= circle_right && point.y >= circle_top && point.y <= circle_bottom) {
            float dx = (circle.x - point.x) * (circle.x - point.x);
            float dy = (circle.y - point.y) * (circle.y - point.y);
            return (dx + dy) <= (radius * radius);
        }
        
        return false;
    }

    // From Phaser
    // Works well and can detect if a line is inside a circle also
    // Nearest is the point closest to the center
    inline bool line_circle(const Vector2 &lineP1, const Vector2 &lineP2, const Vector2 &circle_center, const float &radius, Vector2 &nearest) {
        if (circle_contains_point(circle_center, radius, lineP1)) {
            nearest.x = lineP1.x;
            nearest.y = lineP1.y;
            // furthest.x = lineP2.x;
            // furthest.y = lineP2.y;
            return true;
        }

        if (circle_contains_point(circle_center, radius, lineP2)) {
            nearest.x = lineP2.x;
            nearest.y = lineP2.y;
            // furthest.x = lineP1.x;
            // furthest.y = lineP1.y;
            return true;
        }

        float dx = lineP2.x - lineP1.x;
        float dy = lineP2.y - lineP1.y;

        float lcx = circle_center.x - lineP1.x;
        float lcy = circle_center.y - lineP1.y;

        //  project lc onto d, resulting in vector p
        float dLen2 = (dx * dx) + (dy * dy);
        float px = dx;
        float py = dy;

        if (dLen2 > 0) {
            float dp = ((lcx * dx) + (lcy * dy)) / dLen2;
            px *= dp;
            py *= dp;
        }

        nearest.x = lineP1.x + px;
        nearest.y = lineP1.y + py;
        
        //  len2 of p
        float pLen2 = (px * px) + (py * py);
        return pLen2 <= dLen2 && ((px * dx) + (py * dy)) >= 0 && circle_contains_point(circle_center, radius, nearest);
    }

    // Works good and finds the entry point of collision
    // return values:
    // 0: no collision
    // 1: collision but no entry/exit point
    // 2: collision and entry/exit point closest to segment_start
    inline int line_circle_entry(const Vector2 &segment_start, const Vector2 &segment_end, const Vector2 &center, const float &radius, Vector2 &intersection) {
        // if (circle_contains_point(center, radius, segment_start)) {
        //     return true;
        // }

        // if (circle_contains_point(center, radius, segment_end)) {
        //     return true;
        // }
        
        /*
        Taking

        E is the starting point of the ray,
        L is the end point of the ray,
        C is the center of sphere you're testing against
        r is the radius of that sphere

        Compute:
        d = L - E ( Direction vector of ray, from start to end )
        f = E - C ( Vector from center sphere to ray start ) 
        */
        Vector2 d = segment_end - segment_start;
        Vector2 f = segment_start - center;
        float r = radius;

        float a = d.dot( d ) ;
        float b = 2*f.dot( d ) ;
        float c = f.dot( f ) - r*r ;

        float discriminant = b*b-4*a*c;
        if( discriminant < 0 ) {
            // no intersection
            return 0;
        }
    
        // ray didn't totally miss sphere,
        // so there is a solution to
        // the equation.
        discriminant = Math::sqrt_f( discriminant );

        // either solution may be on or off the ray so need to test both
        // t1 is always the smaller value, because BOTH discriminant and
        // a are nonnegative.
        float t1 = (-b - discriminant)/(2*a);
        float t2 = (-b + discriminant)/(2*a);
        
        // 3x HIT cases:
        //          -o->             --|-->  |            |  --|->
        // Impale(t1 hit,t2 hit), Poke(t1 hit,t2>1), ExitWound(t1<0, t2 hit), 

        // 3x MISS cases:
        //       ->  o                     o ->              | -> |
        // FallShort (t1>1,t2>1), Past (t1<0,t2<0), CompletelyInside(t1<0, t2>1)

        if(t1 <= 0 && t2 >= 1) {
            // Completely inside
            // we consider this a hit, not a miss
            // Engine::logn("inside");
            return 1;
        }

        if(t1 >= 0 && t1 <= 1)
        {
            // t1 is the intersection, and it's closer than t2
            // (since t1 uses -b - discriminant)
            // Impale, Poke
            // Engine::logn("impale, poke");
            intersection = Vector2(segment_start.x + t1 * d.x, segment_start.y + t1 * d.y);
            return 2;
        }

        // here t1 didn't intersect so we are either started
        // inside the sphere or completely past it
        if(t2 >= 0 && t2 <= 1)
        {
            // ExitWound
            // Engine::logn("exit wound");
            intersection = Vector2(segment_start.x + t1 * d.x, segment_start.y + t1 * d.y);
            return 2;
        }

        // no intn: FallShort, Past,  // CompletelyInside
        return 0;    
    }
}

#endif