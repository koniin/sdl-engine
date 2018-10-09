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

struct BaseContainer {
    unsigned length;
    unsigned max_size;

    void create_component() {
        length++;
    }

    virtual void remove_component(unsigned index) = 0;
};

// template<typename T>
// struct ComponentContainer : BaseContainer {
// 	ComponentContainer() {}
// 	ComponentContainer(const std::vector<T> data) : data(data) {}
//     std::vector<T> data;
// };

const size_t CHUNK_SIZE = 128;

template<typename T>
struct ComponentContainer : BaseContainer {
    const int invalid_index = -1;
    T* instances;
    // // AoS
    // struct InstanceData {
    //     // unsigned size;
    //     // unsigned n;
        
    // };
    // InstanceData data;

    void allocate(unsigned size) {
        instances = new T[size];
        length = 0;
        max_size = size;
        //data.size = size;
    }

    void remove_component(unsigned index) override {
        instances[index] = instances[length - 1];
        length--;
    }

    //void create_component(Entity e) {
        // ASSERT_WITH_MSG(data.n <= data.size, "Component storage is full, n:" + std::to_string(data.n));

        // ASSERT_WITH_MSG(_map.find(e.id) != _map.end(), "Entity already has component");
        
        // unsigned int index = data.n;
        // _map[e.id] = index;
        
    //}

    // /// Create an instance from an index to the data arrays.
    // Handle make_handle(int i) {
    //     Handle inst = {i}; 
    //     return inst;
    // }

    // /// Returns the component instance for the specified entity or a nil instance
    // /// if the entity doesn't have the component.
    // Handle lookup(Entity e) {
    //     auto a = _map.find(e.id);
    //     if(a != _map.end()) {
    //         return a->second;
    //     } else {
    //         return invalid_index;
    //     }
    // }

    // bool is_valid(Handle i) {
    //     return i.i > -1;
    // }


    // void remove_component(Handle i) {
    //     const int index = i.i;
    //     const unsigned lastIndex = data.n - 1;

    //     if (is_valid(i) && lastIndex >= 0) {
    //         // Get the entity at the index to destroy
    //         Entity entityToDestroy = data.entity[index];
    //         // Get the entity at the end of the array
    //         Entity lastEntity = data.entity[lastIndex];

            
    //         // Move last entity's data
    //         data.entity[index] = data.entity[lastIndex];
    //         data.instances[index] = data.instances[lastIndex];

    //         // Update map entry for the swapped entity
    //         _map[lastEntity.id] = index;
    //         // Remove the map entry for the destroyed entity
    //         _map.erase(entityToDestroy.id);

    //         // Decrease count
    //         data.n--;
    //     }
    // }

    // T get_component(Handle i) {
    //     return data.instances[i.i];
    // }

    // void set_component(Handle i, T input) {
    //     data.instances[i.i] = input;
    // }
};

struct Store {
    struct ArchetypeRepository {
        size_t count = 0;
        std::unordered_map<ComponentID, BaseContainer*> components;
        
        std::vector<unsigned char> _generation;
        std::queue<unsigned> _free_indices;

        Entity *entities;
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
    };
    
    std::unordered_map<ComponentMask, ArchetypeRepository> archetypes;

    Entity add_entity(ComponentMask mask) {
        Entity e = archetypes[mask].create(mask);
        return e;
    }

    void remove_entity(Entity entity) {
        archetypes[entity.mask].destroy(entity);
    }

    // unsigned int add(ComponentMask m) {
    //     chunks[m].components

    //     unsigned int id = chunks[m].count;
    //     chunks[m].count++;
    //     for(auto c : chunks[m].components) {
    //         c.second->length = chunks[m].count;
    //     }
    //     return id;
    // }

    template <typename C>
    void allocate(ComponentMask mask) {
        auto container = new ComponentContainer<C>();        
        container->allocate(CHUNK_SIZE);
        archetypes[mask].entities = new Entity[CHUNK_SIZE];
        archetypes[mask].components[TypeID::value<C>()] = container;
    }

    template <typename C, typename C2, typename ... Components>
    void allocate(ComponentMask m) {
        allocate<C>(m);
        allocate<C2, Components ...>(m);
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

    Entity create_entity(const EntityArchetype &ea) {
        return storage.add_entity(ea.mask);
    }

    void destroy_entity(Entity entity) {
        storage.remove_entity(entity);
    }

    template<typename T>
	void set_component(const Entity entity, T component) {
        auto &archetype = storage.archetypes[entity.mask];
        auto a = archetype._map.find(entity.id);
        if(a != archetype._map.end()) {
            unsigned index = a->second;
            auto container = static_cast<ComponentContainer<T>*>(archetype.components[TypeID::value<T>()]);
            container->instances[index] = component;
        }
    }
};

template<typename T>
struct ForwardIndexer {
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

	T &index(unsigned i) {
		ASSERT_WITH_MSG(i < length, "index out of bounds");
		
		if(i >= cache.cached_end) {
			update_cache(i);
		}
		return static_cast<T&>(*(cache.cache_ptr + i));
	}

	void update_cache(unsigned i) {
		auto size = cache.datasizes[cache.data_ptr];
		cache.cached_begin += size;
		cache.cached_end += size;
		cache.cache_ptr = (cache.data[++cache.data_ptr] - cache.cached_begin);
	}
};

template<typename ... Components>
struct ComponentIterator {

};

// owns systems / manages systems
// owns entitymanager
struct World {
    EntityManager *entity_manager;

    EntityManager *get_entity_manager() {
        return entity_manager;
    }

    // template<typename Component>
    // void init_indexer(ForwardIndexer<Component> &indexer) {
    //     ComponentMask m = entity_manager->create_mask<Component>();
    //     for(auto &c : entity_manager->storage.archetypes) {
    //         if((c.first & m) == m) {
    //             auto container = static_cast<ComponentContainer<Component>*>(c.second.components[TypeID::value<Component>()]);
    //             indexer.add(container->instances, container->length);
    //             Engine::log("added");
    //         }
    //     }
    // }

    template<typename ... Components>
    void get_iterator(ComponentIterator<Components ...> &iterator) {

    }

    template<typename ... Components, typename Component>
    void init_indexer(ForwardIndexer<Component> &indexer) {
        ComponentMask m = entity_manager->create_mask<Components ...>();
        for(auto &c : entity_manager->storage.archetypes) {
            if((c.first & m) == m) {
                auto container = static_cast<ComponentContainer<Component>*>(c.second.components[TypeID::value<Component>()]);
                indexer.add(container->instances, container->length);
            }
        }
    }

    template<typename ... Components>
    void init_entity_indexer(ForwardIndexer<Entity> &indexer) {
        ComponentMask m = entity_manager->create_mask<Components ...>();
        for(auto &c : entity_manager->storage.archetypes) {
            if((c.first & m) == m) {
                //auto container = static_cast<ComponentContainer<Component>*>(c.second.components[TypeID::value<Component>()]);
                indexer.add(c.second.entities, c.second.count);
            }
        }
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