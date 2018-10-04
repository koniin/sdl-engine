#include "engine.h"

#include <queue>
#include <unordered_map>

// http://bitsquid.blogspot.com/2014/08/building-data-oriented-entity-system.html

const unsigned ENTITY_INDEX_BITS = 22;
const unsigned ENTITY_INDEX_MASK = (1<<ENTITY_INDEX_BITS)-1;

const unsigned ENTITY_GENERATION_BITS = 8;
const unsigned ENTITY_GENERATION_MASK = (1<<ENTITY_GENERATION_BITS)-1;

typedef unsigned EntityId;
struct Entity {
    EntityId id;

    unsigned index() const { return id & ENTITY_INDEX_MASK; }
    unsigned generation() const { return (id >> ENTITY_INDEX_BITS) & ENTITY_GENERATION_MASK; }
};

    // template <>
    // struct Hash<Entity> {
    //     static constexpr uint64 hash(Entity ent) {
    //         return ent.id;
    //     }
    // };

// Don't reuse entity ids before there are more than MINIMUM_FREE_INDICES ids to take from
/* 
    We've split up our 30 bits into 22 bits for the index and 8 bits for the generation. 
    This means that we support a maximum of 4 million simultaneous entities. 
    It also means that we can only distinguish between 256 different entities created 
    at the same index slot. If more than 256 entities are created at the same index slot, 
    the generation value will wrap around and our new entity will get the same ID as an old entity.

    To prevent that from happening too often we need to make sure that we don't reuse 
    the same index slot too often. There are various possible ways of doing that. 
    Our solution is to put recycled indices in a queue and only reuse values from that 
    queue when it contains at least MINIMUM_FREE_INDICES = 1024 items. 
    Since we have 256 generations, an ID will never reappear until its index has run 256 laps 
    through the queue. So this means that you must create and destroy at least 256 * 1024 entities 
    until an ID can reappear. This seems reasonably safe, 
    but if you want you can play with the numbers to get different margins. 
    For example, if you don't need 4 M entities, you can steal some bits from index and 
    give to generation.
*/
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

struct PlayerInput {
    float move_x = 0;
    float move_y = 0;
    bool shoot = false;
};

struct Position {

};

const unsigned MAX_ENTITIES = 4096;

template<typename T>
struct ComponentStore {
    const int invalid_index = -1;
    // AoS
    struct InstanceData {
        unsigned size;
        unsigned n;
        EntityId* entity;
        T* instances;
    };
    /* SoA
    struct InstanceData {
        unsigned n;
        Entity* entity;
        float* move_x;
        float* move_y;
        bool* shoot;
    };*/
    InstanceData data;

    std::unordered_map<EntityId, unsigned> _map;

    void allocate(unsigned size) {
        data.entity = new EntityId[size];
        data.instances = new T[size];
        data.n = 0;
        data.size = size;
    }

    /// Handle to a component instance.
    struct Instance {
        int i;
    };

    /// Create an instance from an index to the data arrays.
    Instance make_instance(int i) {
        Instance inst = {i}; 
        return inst;
    }

    /// Returns the component instance for the specified entity or a nil instance
    /// if the entity doesn't have the component.
    Instance lookup(Entity e) {
        auto a = _map.find(e.id);
        if(a != _map.end()) {
            return make_instance(a->second);
        } else {
            return make_instance(invalid_index);
        }
    }

    bool is_valid(Instance i) {
        return i.i > -1;
    }

    Instance create_component(Entity e) {
        ASSERT_WITH_MSG(data.n <= data.size, "Component storage is full, n:" + std::to_string(data.n));

        auto i = lookup(e);
        ASSERT_WITH_MSG(!is_valid(i), "Entity already has component");
        
        unsigned int index = data.n;
        _map[e.id] = index;
        data.n++;
        return make_instance(index);
    }

    void remove_component(Instance i) {
        const int index = i.index;
        const unsigned lastIndex = data.n - 1;

        if (is_valid(i) && lastIndex >= 0) {
            // Get the entity at the index to destroy
            EntityId entityToDestroy = data.entity[index];
            // Get the entity at the end of the array
            EntityId lastEntity = data.entity[lastIndex];

            // Move last entity's data
            data.entity[index] = data.entity[lastIndex];
            data.component[index] = data.component[lastIndex];

            // Update map entry for the swapped entity
            m_map[lastEntity] = index;
            // Remove the map entry for the destroyed entity
            m_map.erase(entityToDestroy);

            // Decrease count
            data.n--;
        }
    }

    T get_component(Instance i) {
        return data.instances[i.i];
    }

    void set_component(Instance i, T input) {
        data.instances[i.i] = input;
    }

    void gc(const EntityManager &em) {
        unsigned alive_in_row = 0;
        Entity e;
        while (data.n > 0 && alive_in_row < 4) {
            unsigned i = RNG::range_i(0, data.n - 1);
            e.id = data.entity[i];
            if (em.alive(e)) {
                ++alive_in_row;
                continue;
            }
            alive_in_row = 0;
            destroy(i);
        }
    }
};

void simulate(float dt) {
    /*
    for (unsigned i=0; i<_data.n; ++i) {
        _data.velocity[i] += _data.acceleration[i] * dt;
        _data.position[i] += _data.velocity[i] * dt;
    }
    */
}

void test_ecs_bitsquid() {
    Engine::logn("BITSQUID!");

    EntityManager em;    

    Entity test = em.make_entity(45, 248);
    Engine::logn("Test Entity id: %d, index: %d, generation: %d", test.id, test.index(), test.generation());

    Entity e = em.create();
    Engine::logn("Entity id: %d, index: %d, generation: %d", e.id, e.index(), e.generation());
    Entity e2 = em.create();
    Engine::logn("Entity id: %d, index: %d, generation: %d", e2.id, e2.index(), e2.generation());

    Engine::logn("Entity alive: %s", em.alive(e2) ? "yes" : "no");
    em.destroy(e2);
    Engine::logn("Entity alive: %s", em.alive(e2) ? "yes" : "no");

    for(size_t i = 0; i < 1024; i++) {
        auto ee = em.create();
        em.destroy(ee);
    }

    Entity e3 = em.create();
    Engine::logn("Entity id: %d, index: %d, generation: %d", e3.id, e3.index(), e3.generation());

/*
    TEST WITH MORE ENTITIES ADDED
    THEN DO COMPLETE ASTEROIDS BITSQUID STYLE
    THEN WE CAN MAKE IT MORE GENERIC !
*/

    ComponentStore<PlayerInput> pim;
    pim.allocate(234);

    // for(size_t i = 0; i < 1024; i++) {
    //     auto ee = em.create();
    //     pim.create_component(ee);
    //     em.destroy(ee);
    // }

    Engine::logn("PlayerInputManager | n: %d", pim.data.n);

    auto instance = pim.create_component(e3);
    Engine::logn("PlayerInputManager | n: %d", pim.data.n);
    pim.set_component(instance, { 654 });
    PlayerInput p = pim.get_component(instance);
    Engine::logn("Got: %f", p.move_x);

    auto instance1 = pim.create_component(e);
    Engine::logn("PlayerInputManager | n: %d", pim.data.n);

    // pim.gc(em);
}