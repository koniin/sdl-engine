#ifndef ECS_H
#define ECS_H

#include "engine.h"
#include <queue>
#include <bitset>

const size_t MAX_ARCHETYPE_COMPONENTS = 20;

const unsigned ENTITY_INDEX_BITS = 22;
const unsigned ENTITY_INDEX_MASK = (1<<ENTITY_INDEX_BITS)-1;

const unsigned ENTITY_GENERATION_BITS = 8;
const unsigned ENTITY_GENERATION_MASK = (1<<ENTITY_GENERATION_BITS)-1;

typedef std::bitset<MAX_ARCHETYPE_COMPONENTS> ComponentMask;

typedef unsigned EntityId;
struct Entity {
    EntityId id;
    
    ComponentMask mask;

    unsigned generation_index() const { return id & ENTITY_INDEX_MASK; }
    unsigned generation() const { return (id >> ENTITY_INDEX_BITS) & ENTITY_GENERATION_MASK; }
};

const unsigned MINIMUM_FREE_INDICES = 1024;

typedef size_t ComponentID;

class TypeID {
    static ComponentID counter;
public:
    template<typename T>
    static ComponentID value() {
        static ComponentID id = counter++;
        return id;
    }
};
ComponentID TypeID::counter = 1;

struct EntityArchetype {
    ComponentMask mask;
};

struct ComponentContainer {
    void *instances;
    unsigned length;
    unsigned max_size;
    size_t type_size;

    template<typename T>
    void allocate(unsigned size) {
        instances = new T[size];
        length = 0;
        max_size = size;        
        type_size = sizeof(T);
    }

    void create_component() {
        length++;
    }

    void remove_component(unsigned index) {
        std::memcpy((char*)instances + (index * type_size), 
            (char*)instances + (--length * type_size), 
            type_size);
    }
};


const size_t CHUNK_SIZE = 128;

struct Store {
    struct ArchetypeRepository {
        size_t count = 0;
        std::unordered_map<ComponentID, ComponentContainer*> components;
        
        std::vector<unsigned char> _generation;
        std::queue<unsigned> _free_indices;

        Entity *entities = nullptr;
        std::unordered_map<EntityId, unsigned> _map;
        
        Entity create(ComponentMask mask) {
            Entity e = make_entity_id();
            e.mask = mask;
            _map[e.id] = count;
		    entities[count] = e;
            ++count;

            for(auto c : components) {
                c.second->create_component();
                ASSERT_WITH_MSG(c.second->length == count, "something wrong with creating entity");
                // c.second->length = chunks[m].count;
            }
            return e;
        }

        Entity make_entity_id() {
            unsigned idx;
            if (_free_indices.size() > MINIMUM_FREE_INDICES) {
                idx = _free_indices.front();
                _free_indices.pop();
            } else {
                _generation.push_back(0);
                idx = _generation.size() - 1;
                ASSERT_WITH_MSG(idx < (1 << ENTITY_INDEX_BITS), "idx is malformed, larger than 22 bits?");
            }

            return assign_entity_id(idx, _generation[idx]);
        }

        Entity assign_entity_id(unsigned idx, unsigned char generation) {
            Entity e;
            auto id = generation << ENTITY_INDEX_BITS | idx;
            e.id = id;
            return e;
        }

        bool alive(Entity e) const {
            return _generation[e.generation_index()] == e.generation();
        }

        void destroy(Entity e) {
            if(!alive(e))
                return;

            const unsigned idx = e.generation_index();
            ++_generation[idx];
            _free_indices.push(idx);

            // Find the entity by id
            auto a = _map.find(e.id);
            if(a != _map.end()) {
                const int index = a->second;
                const unsigned lastIndex = count - 1;

                Entity entityToDestroy = entities[index];
                Entity lastEntity = entities[lastIndex];

                entities[index] = lastEntity;

                // Remove data
                for(auto c : components) {
                    c.second->remove_component(index);
                }

                _map[lastEntity.id] = index;
                _map.erase(entityToDestroy.id);
                
                count--;
            } 
            // ASSERT_WITH_MSG(0, "destroy is not implemented");
        }

        template<typename T>
        void set_data(const Entity entity, T data) {
            auto a = _map.find(entity.id);
            if(a != _map.end()) {
                unsigned index = a->second;
                auto container = static_cast<T*>(components[TypeID::value<T>()]->instances);
                container[index] = data;
            }
        }
    };
    
    std::unordered_map<ComponentMask, ArchetypeRepository> archetypes;

    Entity add_entity(ComponentMask mask) {
        Entity e = archetypes[mask].create(mask);
        return e;
    }

    void remove_entity(Entity entity) {
        archetypes[entity.mask].destroy(entity);
    }

    template <typename C>
    void allocate(ComponentMask mask) {
        auto container = new ComponentContainer();
        container->allocate<C>(CHUNK_SIZE);

        // Only allocate one array of entities per archetype
        if(!archetypes[mask].components.size()) {
            archetypes[mask].entities = new Entity[CHUNK_SIZE];
        }
        archetypes[mask].components[TypeID::value<C>()] = container;
    }

    template <typename C, typename C2, typename ... Components>
    void allocate(ComponentMask m) {
        allocate<C>(m);
        allocate<C2, Components ...>(m);
    }

    template<typename T>
    void set_component_data(const Entity entity, T component) {
        archetypes[entity.mask].set_data(entity, component);
    }

    size_t count(ComponentMask mask) {
        return archetypes[mask].count;
    }
};

struct EntityManager {
    Store storage;

    template <typename C>
    EntityArchetype create_archetype() {
        EntityArchetype a;
        a.mask = create_mask<C>();
        storage.allocate<C>(a.mask);
        return a;
    }

    template <typename C1, typename C2, typename ... Components>
    EntityArchetype create_archetype() { 
        EntityArchetype a;
        a.mask = create_mask<C1, C2, Components ...>();
        storage.allocate<C1, C2, Components ...>(a.mask);
        return a;
    }

    template <typename C>
    ComponentMask create_mask() {
        ComponentMask mask;
        mask.set(TypeID::value<C>());
        return mask;
    }

    template <typename C1, typename C2, typename ... Components>
    ComponentMask create_mask() {
        return create_mask<C1>() | create_mask<C2, Components ...>();
    }

    size_t archetype_count(const EntityArchetype &ea) {
        return storage.count(ea.mask);
    }

    Entity create_entity(const EntityArchetype &ea) {
        return storage.add_entity(ea.mask);
    }

    void destroy_entity(Entity entity) {
        storage.remove_entity(entity);
    }

    template<typename T>
	void set_component(const Entity entity, T component) {
        storage.set_component_data(entity, component);
    }
};

template<typename T>
struct ComponentArray {
	unsigned length = 0;
	
	struct Cache {
		T *cache_ptr;
		unsigned cached_begin = 0;
		unsigned cached_end = 0;
		int data_n = 0;
		int data_ptr = 0;
		T *data[1024];
		size_t datasizes[1024];
	} cache;

    inline T operator [](int i) const { return index(i); }
    inline T & operator [](int i) { return index(i); }

	void add(T* data, unsigned n) {
		if(cache.cached_end == 0) {
			cache.cached_begin = 0;
			cache.data_ptr = 0;
			cache.cached_end = n;
		}
		cache.datasizes[cache.data_n] = n;
		cache.data[cache.data_n++] = data;
		cache.cache_ptr = cache.data[cache.data_ptr];
		length += n;
	}


    int last_index = -1;
	T &index(unsigned i) {
		ASSERT_WITH_MSG(i >= 0 && i < length, "index out of bounds");
		ASSERT_WITH_MSG(last_index <= (int)i, "FORWARD ITERATION ONLY");
        
        last_index = i;

		if(i >= cache.cached_end) {
			update_cache(i);
		}
		return static_cast<T&>(*(cache.cache_ptr + i));
	}

	void update_cache(unsigned i) {
		cache.cached_begin += cache.datasizes[cache.data_ptr];
		cache.cache_ptr = (cache.data[++cache.data_ptr] - cache.cached_begin);
        cache.cached_end +=  cache.datasizes[cache.data_ptr];
	}

    void clear() {
        length = 0;
    }

    void reset() {
        cache.cached_begin = 0;
		cache.data_ptr = 0;
		cache.cached_end = cache.datasizes[cache.data_ptr];
        cache.cache_ptr = cache.data[cache.data_ptr];
        last_index = -1;
    }
};

template<typename ... Components>
struct ComponentData {
    unsigned length;
};

template<typename ... Components>
struct EntityComponentData : ComponentData<Components...> {
    ComponentArray<Entity> entities;
};

// owns systems / manages systems
// owns entitymanager
struct World {
    EntityManager *entity_manager;

    EntityManager *get_entity_manager() {
        return entity_manager;
    }
    
    typedef int expander[];

    template<typename ... Components, typename ... Iterators>
    void fill_data(ComponentData<Components...> &data, ComponentArray<Iterators> &... iterators) {
        expander { 0, ( (void) fill_length<Components...>(data.length, iterators), 0) ... };
    }

    template<typename ... Components, typename ... Iterators>
    void fill_entity_data(EntityComponentData<Components...> &data, ComponentArray<Entity> &entities, ComponentArray<Iterators> &... iterators) {
        expander { 0, ( (void) fill_length<Components...>(data.length, iterators), 0) ... };
        fill_entities<Components...>(entities);
        data.length = entities.length;
    }

    // Use to fill a bunch of componentarrays
    // Will fill components based on the mask of arrays passed in
    // if you pass a Position, Velocity and X array it will find all archetypes 
    // matching Position, Velocity and X
    // e.g fill_by_type(l, ComponentArray<Position>, ComponentArray<Velocity>)
    template<typename ... Components>
    void fill_by_arguments(unsigned &length, ComponentArray<Components> &... iterators) {
        expander { 0, ( (void) fill_length<Components...>(length, iterators), 0) ... };
    }

    // Fills the arrays based on the mask defined as types
    // fill_by_types<Velocity, Position, Faction, MoveForwardComponent>(length, vv, pp);
    // this will fill the vv and pp arrays with all components of that type that 
    // matches the mask => Velocity, Position, Faction, MoveForwardComponent
    template<typename ... Components, typename ... Iterators>
    void fill_by_types(unsigned &length, ComponentArray<Iterators> &... iterators) {
        expander { 0, ( (void) fill_length<Components...>(length, iterators), 0) ... };
    }
    
    template<typename ... Components>
    void fill_entities(ComponentArray<Entity> &indexer) {
        ComponentMask m = entity_manager->create_mask<Components ...>();
        for(auto &c : entity_manager->storage.archetypes) {
            if((c.first & m) == m && c.second.count > 0) {
                indexer.add(c.second.entities, c.second.count);
            }
        }
    }

    template<typename ... Components, typename Component>
    void fill(ComponentArray<Component> &indexer) {
        ComponentMask m = entity_manager->create_mask<Components ...>();
        for(auto &c : entity_manager->storage.archetypes) {
            if((c.first & m) == m && c.second.count > 0) {
                auto container = c.second.components[TypeID::value<Component>()];
                indexer.add(static_cast<Component*>(container->instances), container->length);
            }
        }
    }
    
    private:
        template<typename ... Components, typename Component>
        void fill_length(unsigned &length, ComponentArray<Component> &indexer) {
            ComponentMask m = entity_manager->create_mask<Components ...>();
            for(auto &c : entity_manager->storage.archetypes) {
                if((c.first & m) == m && c.second.count > 0) {
                    auto container = c.second.components[TypeID::value<Component>()];
                    indexer.add(static_cast<Component*>(container->instances), container->length);
                }
            }
            length = indexer.length;
        }
};

World *make_world() {
    World *w = new World;
    w->entity_manager = new EntityManager;
    return w;
}


#include <queue>

struct EventQueue {
	struct Evt {
		void *data;
		size_t type;
		static size_t counter;
		template<typename T>
		bool is() {
			return getType<T>() == type;
		}

		template<typename T>
		void set() {
			type = getType<T>();
		}

		template<typename T>
		T *get() {
			return static_cast<T*>(data);
		}

		void destroy() {
			delete data;
		}

		template<typename T>
		static size_t getType() {
			static size_t id = counter++;
			return id;
		}
	};

	std::vector<Evt> events;

	template<typename T>
	void queue_evt(T *data) {
		Evt evt = { data };
		evt.set<T>();
		events.push_back(evt);
	}

    void clear() {
        for(auto &e : events) {
            e.destroy();
        }
        events.clear();
    }
};
size_t EventQueue::Evt::counter = 1;

#endif