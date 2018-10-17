#ifndef ECS_H
#define ECS_H

#include "engine.h"

#include <bitset>
#include <unordered_map>
#include <typeindex>

struct BaseContainer {
    size_t length;
};

template<typename T>
struct ComponentContainer : BaseContainer {
	ComponentContainer() {}
	ComponentContainer(const std::vector<T> data) : data(data) {}
    std::vector<T> data;
};

static const size_t MAX_ARCHETYPE_ENTITIES = 128;
static const size_t MAX_ARCHETYPE_COMPONENTS = 40;
typedef std::bitset<MAX_ARCHETYPE_COMPONENTS> ComponentMask;

struct Entity {
    int id;
    ComponentMask mask;
};

struct EntityArcheType {
    ComponentMask mask;
};

typedef size_t ComponentID;

class TypeID
{
    static ComponentID counter;
public:
    template<typename T>
    static ComponentID value()
    {
        static ComponentID id = counter++;
        return id;
    }
};
ComponentID TypeID::counter = 1;

struct Chunk {
    size_t count = 0;
    std::unordered_map<ComponentID, BaseContainer*> components;

    template<typename T>
    ComponentContainer<T> *get_container() {
        return static_cast<ComponentContainer<T>*>(components[TypeID::value<T>()]);
    }

    template<typename T>
    T &get(size_t i) {
        return static_cast<ComponentContainer<T>*>(components[TypeID::value<T>()])->data[i];
    }
    
    template <typename ... Args, typename F>
    void call_function(F f, size_t i)  {
        f(get<Args>(i)...);
    }

    template <typename... Args>
    std::tuple<Args...> get_components(size_t i) {
        return std::make_tuple(get<Args>(i)...);
    }

};

struct Store {
    std::unordered_map<ComponentMask, Chunk> chunks;
    
    unsigned int add(ComponentMask m) {
        unsigned int id = chunks[m].count;
        chunks[m].count++;
        for(auto c : chunks[m].components) {
            c.second->length = chunks[m].count;
        }
        return id;
    }

    template <typename C>
    void allocate(ComponentMask m) {
        // Here we allocate memory and data
        std::vector<C> data(MAX_ARCHETYPE_ENTITIES);

        /*
        You can do this instead but then you need create data on adding to vector
        std::vector<C> data();
        data.reserve(MAX_ARCHETYPE_ENTITIES);
        C c; 
        c.XX = ;
        data[i] = c;
        */
        auto container = new ComponentContainer<C>(data);
        container->length = chunks[m].count;
        chunks[m].components[TypeID::value<C>()] = container;
        //Engine::log("\n Allocating for: %d", TypeID::value<C>());
    }

    template <typename C, typename C2, typename ... Components>
    void allocate(ComponentMask m) {
        allocate<C>(m);
        allocate<C2, Components ...>(m);
    }
};

struct EntityManager {
    unsigned int type_count = 0;
    Store storage;

    template <typename C>
    EntityArcheType create_archetype() {
        EntityArcheType a;
        a.mask = create_mask<C>();
        storage.allocate<C>(a.mask);
        return a;
    }

    template <typename C1, typename C2, typename ... Components>
    EntityArcheType create_archetype() { 
        EntityArcheType a;
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

    Entity create_entity(const EntityArcheType &ea) {
        unsigned int id = storage.add(ea.mask);
        Entity entity;
        entity.id = id;
        entity.mask = ea.mask;
        return entity;
    }

    template<typename T>
	void set_component_data(const Entity entity, T component) {
        auto &chunk = storage.chunks[entity.mask];
        auto container = static_cast<ComponentContainer<T>*>(chunk.components[TypeID::value<T>()]);
        container->data[entity.id] = component;
    }

    template<typename T>
    T get_component_data(const Entity entity) {
        auto &chunk = storage.chunks[entity.mask];
        auto container = static_cast<ComponentContainer<T>*>(chunk.components[TypeID::value<T>()]);
        return container->data[entity.id];
    }

    template<typename T>
    bool has_component(const Entity entity) {
        return entity.mask.test(TypeID::value<T>());
    }

    bool exists(const Entity entity) {
        ASSERT_WITH_MSG(false, "exists IS NOT IMPLEMENTED");
        //return false;
    }
    
    template<typename ... Components, typename F>
    void each(F f) {
        ComponentMask m = create_mask<Components ...>();
        for(auto &c : storage.chunks) {
            if((c.first & m) == m) {
                auto &chunk = c.second;
                for(size_t i = 0; i < chunk.count; i++) {
                    chunk.call_function<Components...>(f, i);
                }
            }
        }
    }
};

template<typename Component>
struct ContainerIterator {
    std::vector<std::tuple<typename std::vector<Component>::iterator, typename std::vector<Component>::iterator, size_t>> iterators;
    size_t count = 0;
    size_t current_length = 0;
    size_t index = -1;
    size_t current_vector = 0;
    typename std::vector<Component>::iterator it;
    typename std::vector<Component>::iterator itend;

    void add(ComponentContainer<Component> *container) {
        size_t length = container->length;
        Engine::log("\n Size: %d", length);
        if(length == 0)
            return;
 
        iterators.push_back(std::make_tuple(container->data.begin(), container->data.end(), length));
        if(count == 0) {
            current_length = length;
            it = container->data.begin();
            itend = container->data.end();
        }
        count++;
    }

    bool move_next() {
        if(index != -1)
            it++;
        
        index++;

        if(it == itend || index >= current_length) {
            current_vector++;
            if(current_vector >= count)
                return false;
            
            index = 0;
            it = std::get<0>(iterators[current_vector]);
            itend = std::get<1>(iterators[current_vector]);
            current_length = std::get<2>(iterators[current_vector]);
        }
        return true;
    }

    Component &current() {
        return *it;
    }
};

template<typename Component>
struct IndexedComponentIterator {
    size_t length = 0;
    size_t current_offset = 0;
    size_t container_count = 0;
    size_t current_length = 0;
    ComponentContainer<Component>* current;
    ComponentContainer<Component>* containers[128];

    void add(ComponentContainer<Component>* container) {
        size_t size = container->length;
        if(size == 0)
            return;

        length += size;

        containers[container_count] = container;
        if(container_count == 0) {
            current = container;
            current_length = size;
        }
        container_count++;
    }

    Component &get(size_t index) {
        ASSERT_WITH_MSG(index >= 0 && index < length, "INDEX IS OUTSIDE RANGE!");

        size_t local_index = index - current_offset;
        if(local_index < 0 || local_index >= current_length) {
            current_offset = 0;
            for(size_t i = 0; i < container_count; i++) {
                size_t s = containers[i]->length;
                if(index - current_offset < s) {
                    current = containers[i];
                    current_length = current->length;
                    break;
                } else {
                    current_offset += s;
                }
            }
            local_index = index - current_offset;
        }
        return current->data[local_index];
    }
};

////////////////////////////////////////////////////////
// Game code
////////////////////////////////////////////////////////

struct PlayerInput {
    Vector2 move;
};

struct Position {
    float x;
};

struct Velocity { 
    float x;
};

struct Faction {
    int faction;
};

EntityManager e;
EntityArcheType player_archetype;

void test_iterator() {
    Engine::log("\n Testing iterator system");
    ComponentMask m = e.create_mask<PlayerInput, Position>();

    IndexedComponentIterator<PlayerInput> ici;
    IndexedComponentIterator<Position> pici;
    
    for(auto &c : e.storage.chunks) {
        if((c.first & m) == m) {
            ici.add(c.second.get_container<PlayerInput>());
            pici.add(c.second.get_container<Position>());
        }
    }

    for(size_t i = 0; i < ici.length; i++) {
        Engine::log("\n ICI move.x: %f", ici.get(i).move.x);
        Engine::log("\n ICI position.x: %f", pici.get(i).x);
        ici.get(i).move.x = 42;
    }

    m = e.create_mask<PlayerInput>();
    ContainerIterator<PlayerInput> player_input;
    for(auto &c : e.storage.chunks) {
        if((c.first & m) == m) {
            auto container = c.second.get_container<PlayerInput>();
            player_input.add(container);
        }
    }

    while(player_input.move_next()) {
        Engine::log("\n movex in iterator: %f", player_input.current().move.x);
    }
}

void setup() {
    Engine::log("\n Create Entity Manager");
    // var entityManager = World.Active.GetOrCreateManager<EntityManager>();
    // create entityManager from world or something
    
    Engine::log("\n Make archetype");
    // PlayerArchetype = entityManager.CreateArchetype(
    //     typeof(Position2D), typeof(Heading2D), typeof(PlayerInput),
    //     typeof(Faction), typeof(Health), typeof(TransformMatrix));
    Engine::log("\n\t Creating player archetype | PlayerInput, Position");
    player_archetype = e.create_archetype<PlayerInput, Position>();
    Engine::log("\n\t Creating other archetype | PlayerInput, Velocity");
    auto eaa = e.create_archetype<PlayerInput, Velocity>();
}

void new_game() {    
    Engine::log("\n Create Entity");
    // Access the ECS entity manager
    // var entityManager = World.Active.GetOrCreateManager<EntityManager>();
    // Entity player = entityManager.CreateEntity(PlayerArchetype);
    Engine::log("\n\t Creating player entity");
    auto player = e.create_entity(player_archetype);
    
    Engine::log("\n Set some data on components for entity");
    // // We can change a few components so it makes more sense like this:
    // entityManager.SetComponentData(player, new Position2D { Value = new float2(0.0f, 0.0f) });
    // entityManager.SetComponentData(player, new Heading2D  { Value = new float2(0.0f, 1.0f) });
    // entityManager.SetComponentData(player, new Faction { Value = Faction.Player });
    // entityManager.SetComponentData(player, new Health { Value = Settings.playerInitialHealth });
    Engine::log("\n\t Setting position to 666 for player entity");
    Position p = { 666 };
    e.set_component_data<Position>(player, p);
    // Position p2 = { 321 };
    // e.set_component_data<Position>(player2, p2);
    // Position pOut = e.get_component_data<Position>(player);
    // Engine::log("\n Player pos: %f", pOut.x);
    // Position pOut2 = e.get_component_data<Position>(player2);
    // Engine::log("\n Player TWO pos: %f", pOut2.x);
    
    Engine::log("\n Instantiate PlayerInputSystem");
    Engine::log("\n Instantiate PlayerMoveSystem");
}

void test_iterate_player_input_system() {
    Engine::log("\n\n iterating player input system");

    // We want to get all components we are interrested in => PlayerInput

    // Go through all archetypes matching the mask we are interrested in

    //ComponentMask m = e.create_mask<PlayerInput>();
    // ci.init(m);
    // for(size_t i = 0; i < ci.n; i++) {
    //     Engine::log("\n  iterator,   move x: %f", ci.index(i));
    // }

    Engine::log("\n\n Each PlayerInput");
    e.each<PlayerInput>([](auto &p) {
        Engine::log("\n\t Player Input only: %f", p.move.x);
        Engine::log("\n\t Setting position to 546 for player entity");
        p.move.x = 546;
    });

    test_iterator();

    Engine::log("\n\n each PlayerInput && Position");
    e.each<PlayerInput, Position>([](PlayerInput &p, Position &pos) {
        Engine::log("\n Double component => move.x: %f,  pos.x: %f", p.move.x, pos.x);
    });

    Engine::log("\n\n each PlayerInput && Position && Velocity");
    e.each<PlayerInput, Position, Velocity>([](PlayerInput &p, Position &pos, auto &v) {
        Engine::log("\n EACH => move.x: %f,  pos.x: %f, vel.x: %f", p.move.x, pos.x, v.x);
    });

/*
    for(auto &c : e.storage.chunks) {
        Engine::log("\n chunk: ");
        if((c.first & m) == m) {
            auto &chunk = c.second;
            auto container = static_cast<ComponentContainer<PlayerInput>*>(chunk.components[TypeID::value<PlayerInput>()]);
            Engine::log("MATCH! | entity count: %d", chunk.count);
            for(size_t i = 0; i < chunk.count; i++) {
                Engine::log("\n    move x: %f", container->data[i].move.x);
            }
        }
    }*/
}

void run_game_tick() {
    test_iterate_player_input_system();

    Engine::log("\n Update PlayerInputSystem");
    Engine::log("\n - update all entities with the PlayerInput component");
    Engine::log("\n Update PlayerMoveSystem");
    Engine::log("\n - update all entities with the PlayerInput AND Position component");
    // position += dt * playerInput.Move * settings.playerMoveSpeed;
}

void test_ecs() {
    Engine::log("\n ECS TEST \n ----------- \n");

    setup();

    new_game();

    run_game_tick();

    // EntityArcheType arch;
    // auto pins = new PlayerInput[100];
    // auto cins = new ComponentContainer<PlayerInput>(*pins);
    // arch.components[getTypeIndex<PlayerInput>()] = cins;

    // // Fetches all components of the current type
    // auto input = arch.get<PlayerInput>();
    // input[0].move.x = 23;
    // input[1].move.x = 66;
    // auto input2 = arch.get<PlayerInput>();
    // Engine::log("\n input again: %f \n", input[0].move.x);
    // Engine::log("\n input again again: %f \n", input[1].move.x);
}


/* 
// for(size_t i = 0; i < player_archetype.mask.size(); i++) {
    //     if(player_archetype.mask.test(i)) {
    //         Engine::log("\n %d bit is set", i);
    //     }
    // }
*/

/*

template <typename ... Components>

// template <typename C>
//   ComponentMask component_mask(const ComponentHandle<C> &c) {
//     return component_mask<C>();
//   }

//   template <typename C1, typename ... Components>
//   ComponentMask component_mask(const ComponentHandle<C1> &c1, const ComponentHandle<Components> &... args) {
//     return component_mask<C1, Components ...>();
// }

UnpackingView<Components...> entities_with_components(ComponentHandle<Components> & ... components) {
    auto mask = component_mask<Components...>();
    return UnpackingView<Components...>(this, mask, components...);
}

template<typename T, typename... Args>
	ComponentHandle<T> Entity::assign(Args&&... args)
	{
		using ComponentAllocator = std::allocator_traits<World::EntityAllocator>::template rebind_alloc<Internal::ComponentContainer<T>>;

		auto found = components.find(getTypeIndex<T>());
		if (found != components.end())
		{
			Internal::ComponentContainer<T>* container = reinterpret_cast<Internal::ComponentContainer<T>*>(found->second);
			container->data = T(args...);

			auto handle = ComponentHandle<T>(&container->data);
			world->emit<Events::OnComponentAssigned<T>>({ this, handle });
			return handle;
		}
		else
		{
			ComponentAllocator alloc(world->getPrimaryAllocator());

			Internal::ComponentContainer<T>* container = std::allocator_traits<ComponentAllocator>::allocate(alloc, 1);
			std::allocator_traits<ComponentAllocator>::construct(alloc, container, T(args...));

			components.insert({ getTypeIndex<T>(), container });

			auto handle = ComponentHandle<T>(&container->data);
			world->emit<Events::OnComponentAssigned<T>>({ this, handle });
			return handle;
		}
	}

	template<typename T>
	ComponentHandle<T> Entity::get()
	{
		auto found = components.find(getTypeIndex<T>());
		if (found != components.end())
		{
			return ComponentHandle<T>(&reinterpret_cast<Internal::ComponentContainer<T>*>(found->second)->data);
		}
	
		return ComponentHandle<T>();
}

*/
#endif
