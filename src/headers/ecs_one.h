#ifndef ECS_ONE_H
#define ECS_ONE_H

#include <vector>
#include <unordered_map>

struct ComponentCounter {
  static int counter;
};
int ComponentCounter::counter = 0;

template<typename ComponentType>
struct Component {
  // Get the family for the component
  static inline int family() {
    static int family = ComponentCounter::counter++;
    return family;
  }
};

// Special method to get the id given a type
template <typename C>
static int GetComponentFamily() {
	return Component<typename std::remove_const<C>::type>::family();
}

typedef unsigned EntityID;
typedef unsigned int EntityArchetypeMask;

struct Entity {
	EntityID id;
	EntityArchetypeMask mask;
};

struct EntityArchetypeData {
	int count = 0;
	EntityID entities[100];
	std::unordered_map<EntityArchetypeMask, void*> components;
};

struct EntityArchetype {
	EntityArchetypeMask mask = 0;
	std::vector<int> families;
	
	template<typename ComponentType>
	void addComponent() {
		EntityArchetypeMask family = Component<ComponentType>::family();
		mask |= (1 << family);
		families.push_back(family);
	}
};

class EntityManager {
	public:
		Entity create_entity(EntityArchetype &archetype);
		template<typename T>
		void set_component_data(Entity entity, T component);

		template<typename T>
		EntityArchetype create_archetype();
		template<typename T, typename T2>
		EntityArchetype create_archetype();
		template<typename T, typename T2, typename T3>
		EntityArchetype create_archetype();

		std::unordered_map<EntityArchetypeMask, EntityArchetypeData> archetype_data;
	private:
		unsigned int entity_counter = 0;
		unsigned int next_entity_id() {
			return entity_counter++;
		}

};

Entity EntityManager::create_entity(EntityArchetype &archetype) {
	Entity e = { next_entity_id(), archetype.mask };
	//ASSERT_WITH_MSG(archetype_data.find(archetype.mask) == archetype_data.end(), "Archetype must be created before adding entities to it");

	EntityArchetypeData &d = archetype_data[archetype.mask];
	d.entities[d.count] = e.id;
	d.count++;
	return e;
}

template<typename ComponentType>
void EntityManager::set_component_data(Entity entity, ComponentType data) {
	auto &arch_data = archetype_data[entity.mask];
	for(int i = 0; i < arch_data.count; i++) {
		if(arch_data.entities[i] == entity.id) {
			ComponentType* kk = static_cast<ComponentType*>(arch_data.components[Component<ComponentType>::family()]);
			kk[i] = data;
		}
	}
}

template<typename T>
EntityArchetype EntityManager::create_archetype() {
	EntityArchetype ea;
	ea.addComponent<T>();

	EntityArchetypeData da;
	da.components[Component<T>::family()] = (void*)new T[100];
	archetype_data[ea.mask] = da;
	return ea;
}

template<typename T, typename T2>
EntityArchetype EntityManager::create_archetype() {
	EntityArchetype ea;
	ea.addComponent<T>();
	ea.addComponent<T2>();
	
	EntityArchetypeData da;
	da.components[Component<T>::family()] = (void*)new T2[100];
	da.components[Component<T2>::family()] = (void*)new T2[100];
	archetype_data[ea.mask] = da;
	return ea;
}

template<typename T, typename T2, typename T3>
EntityArchetype EntityManager::create_archetype() {
	EntityArchetype ea;
	ea.addComponent<T>();
	ea.addComponent<T2>();
	ea.addComponent<T3>();
	
	EntityArchetypeData da;
	da.components[Component<T>::family()] = (void*)new T[100];
	da.components[Component<T2>::family()] = (void*)new T2[100];
	da.components[Component<T3>::family()] = (void*)new T3[100];
	archetype_data[ea.mask] = da;
	return ea;
}

struct PlayerInput_one : Component<PlayerInput> {
	float some_number = 0;
};

struct Position_one : Component<Position> {
	float x = 0;
	float y = 0;
};

struct Velocity_one : Component<Velocity> {
    float x = 0;
    float y = 0;
};

EntityManager e;

void system_test(EntityArchetype arch) {
	EntityArchetypeMask mask = arch.mask;
	Engine::log("\n system test, mask: %d matches: ", mask);
	// Loop through all archetypes
	for (std::pair<EntityArchetypeMask, EntityArchetypeData> element : e.archetype_data) {
		// find if the archetype matches the mask
		if((element.first & mask) == mask) {
			Engine::log("\n matched: %d", element.first);

			// loop all components
			auto &arch_data = e.archetype_data[element.first];
			for(int i = 0; i < arch_data.count; i++) {
				for(int i = 0; i < arch.families.size(); i++) {

				}
			}
			
			// Engine::log("\n data? %d", e.archetype_data[element.first]
		}
	}
}

void test_ecs_one() {
	EntityArchetype playerArchetype;
	Engine::log("\n mask: %d \n", playerArchetype.mask);
	playerArchetype = e.create_archetype<PlayerInput_one>();
	Engine::log("\n mask: %d \n", playerArchetype.mask);

	EntityArchetype two;
	Engine::log("\n mask: %d \n", two.mask);
	two = e.create_archetype<PlayerInput_one, Position_one>();
	Engine::log("\n mask: %d \n", two.mask);

	Entity entityTwo = e.create_entity(two);

	Position_one two_p;
	two_p.x = 100;
	two_p.y = 100;
	e.set_component_data(entityTwo, two_p);

	system_test(playerArchetype);
	system_test(two);
}

#endif