#ifndef ECS_STATIC_H
#define ECS_STATIC_H

#include "engine.h"

typedef unsigned EntityID;

struct PlayerInput {
	float some_number = 0;
};

struct Position {
	float x = 0;
	float y = 0;
};

struct Velocity {
    float x = 0;
    float y = 0;
};

// archetype
// - a set of components (like PlayerInput,Position,Velocity) as "id"
struct PlayerInputArchetype {
	unsigned n = 0;
	EntityID *entities;
	PlayerInput *inputs;
};

struct PlayerInputVelocityArchetype {
	unsigned n = 0;
	EntityID *entities;
	PlayerInput *inputs;
	Velocity *velocities;
};

PlayerInputArchetype arch_player_input;
PlayerInputVelocityArchetype arch_player_input_velocity;

static EntityID _current = 0;

EntityID create_entity(PlayerInputArchetype &arch) {
	EntityID id = ++_current;
	arch.entities[arch.n] = id;
	arch.n++;
	Engine::log("\nPlayerInputArchetype.n: %d", arch.n);
    for(unsigned i = 0; i < arch.n; i++) {
        Engine::log("\narch.entities[%d]: %d", i, arch.entities[i]);    
    }
	return id;
}

EntityID create_entity(PlayerInputVelocityArchetype &arch) {
	EntityID id = ++_current;
	arch.entities[arch.n] = id;
	arch.n++;
	Engine::log("\nPlayerInputVelocityArchetype.n: %d", arch.n);
    for(unsigned i = 0; i < arch.n; i++) {
        Engine::log("\narch.entities[%d]: %d", i, arch.entities[i]);    
    }
	return id;
}

void set_component_data(EntityID id, PlayerInput p) {
	for(unsigned i = 0; i < arch_player_input.n; i++) {
		if(arch_player_input.entities[i] == id) {
			arch_player_input.inputs[i] = p;
			return;
		}
	}
	for(unsigned i = 0; i < arch_player_input_velocity.n; i++) {
		if(arch_player_input_velocity.entities[i] == id) {
			arch_player_input_velocity.inputs[i] = p;
			return;
		}
	}
    Engine::log("\n [WARNING] No data set! EntityID: %d", id);
}

PlayerInput &get_component_data(EntityID id) {
    for(unsigned i = 0; i < arch_player_input.n; i++) {
		if(arch_player_input.entities[i] == id) {
			return arch_player_input.inputs[i];
		}
	}
	for(unsigned i = 0; i < arch_player_input_velocity.n; i++) {
		if(arch_player_input_velocity.entities[i] == id) {
			return arch_player_input_velocity.inputs[i];
		}
	}

    ASSERT_WITH_MSG(false, "Check if entity has component first");
}

void allocate_archetypes() {
	unsigned MAX_ENTITIES = 100;
	arch_player_input.entities = new EntityID[MAX_ENTITIES];
	arch_player_input_velocity.entities = new EntityID[MAX_ENTITIES];

	arch_player_input.inputs = new PlayerInput[MAX_ENTITIES];
	arch_player_input_velocity.inputs = new PlayerInput[MAX_ENTITIES];

	arch_player_input_velocity.velocities = new Velocity[MAX_ENTITIES];

	arch_player_input.n = 0;
	arch_player_input_velocity.n = 0;
}

void system_player_inputs(PlayerInputArchetype &arch, PlayerInputVelocityArchetype &arch2) {
	float newValue = 1;
	for(int i = 0; i < arch.n; i++) {
		arch.inputs[i].some_number += newValue;
	}
	for(int i = 0; i < arch2.n; i++) {
		arch2.inputs[i].some_number += newValue;
	}
}

void system_velocities(PlayerInputVelocityArchetype &arch) {
	for(int i = 0; i < arch.n; i++) {
		arch.velocities[i].x += arch.inputs[i].some_number / 100;
	}
}

class ComponentSystem {
    public:
        virtual void update() = 0;
};

class PlayerInputSystem : ComponentSystem {
    public:
        void update() override;
};

void PlayerInputSystem::update() {
    float dt = Time::deltaTime;
    
    for (int i = 0; i < arch_player_input.n; ++i) {
        arch_player_input.inputs[i].some_number = Input::key_pressed(SDLK_u) ? 1 : 0;
    }
}

void test_static() {
    allocate_archetypes();
    
    Engine::log("\n Create Entities\n");
	EntityID player = create_entity(arch_player_input);
	EntityID doubleplayer = create_entity(arch_player_input_velocity);
	EntityID doubleplayer2 = create_entity(arch_player_input_velocity);
    
    Engine::log("\n Set Component Data\n");
	PlayerInput pi = { 66 };
	Velocity v = { 100, 100 };
    set_component_data(player, pi);
    set_component_data(doubleplayer, { 33 });
    set_component_data(doubleplayer2, { 44 });

    PlayerInput p = get_component_data(doubleplayer);
    Engine::log("\n doubleplayer PlayerInput.somenumber: %f\n", p.some_number);

    Engine::log("\n Iterate some systems\n");
    system_player_inputs(arch_player_input, arch_player_input_velocity);
	system_velocities(arch_player_input_velocity);

    Engine::log("\narch_player_input some_number0: %f", arch_player_input.inputs[0].some_number);
	Engine::log("\narch_player_input_velocity some_number0: %f", arch_player_input_velocity.inputs[0].some_number);
	Engine::log("\narch_player_input_velocity some_number1: %f\n", arch_player_input_velocity.inputs[1].some_number);

	for(unsigned i = 0; i <  arch_player_input.n; i++) {
		Engine::log("\n Entity: %d", arch_player_input.entities[i]);
	}
	for(unsigned i = 0; i <  arch_player_input_velocity.n; i++) {
		Engine::log("\n Entity: %d", arch_player_input_velocity.entities[i]);
	}
}


/*
public class PlayerInputSystem : ComponentSystem
    {
        struct PlayerData
        {
            public readonly int Length;

            public ComponentDataArray<PlayerInput> Input;
        }

        [Inject] private PlayerData m_Players;

        protected override void OnUpdate()
        {
            float dt = Time.deltaTime;

            for (int i = 0; i < m_Players.Length; ++i)
            {
                UpdatePlayerInput(i, dt);
            }
        }

        private void UpdatePlayerInput(int i, float dt)
        {
            PlayerInput pi;

            pi.Move.x = Input.GetAxis("Horizontal");
            pi.Move.y = 0.0f;
            pi.Move.z = Input.GetAxis("Vertical");
            pi.Shoot.x = Input.GetAxis("ShootX");
            pi.Shoot.y = 0.0f;
            pi.Shoot.z = Input.GetAxis("ShootY");
            pi.FireCooldown = Mathf.Max(0.0f, m_Players.Input[i].FireCooldown - dt);

            m_Players.Input[i] = pi;
        }
}
*/
#endif