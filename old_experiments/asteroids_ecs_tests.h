void test() {
    spawn_bullet({ Vector2(666, 555) }, { Vector2(222, 333) }, 1);
    spawn_bullet({ Vector2(777, 444) }, { Vector2(111, 12) }, 1);

    // OLD WAY
    Engine::logn("old way:");
    ComponentArray<Position> fp;
    world->init_indexer<Velocity, Position>(fp);

    for(unsigned i = 0; i < fp.length; ++i) {
        const Position &p = fp.index(i);
        Engine::logn("Position: %f", p.value.x);
    }

    Engine::logn("\n TestSome");
    struct TestSome {
        ComponentArray<PlayerInput> fpi;
        ComponentArray<Position> fp;
        ComponentArray<Velocity> fv;
        ComponentArray<Direction> fd;
        ComponentArray<Entity> entities;
        unsigned length;
    } d;

    world->init_entity_indexer<PlayerInput, Position, Velocity, Direction>(d.entities);
    world->update(d.length, d.fpi, d.fp, d.fv, d.fd);
    for(unsigned i = 0; i < d.length; ++i) {
		//PlayerInput &pi = d.fpi.index(i);
        Position &position = d.fp.index(i);
        //Velocity &velocity = d.fv.index(i);
        //Direction &direction = d.fd.index(i);
        //Engine::logn("Entity: %d", d.entities.index(i));
        Engine::logn("Position: %f", position.value.x);
    }

    Engine::logn("\n TestSomePositions");
    struct TestSomePositions {
        ComponentArray<Position> fp;
        ComponentArray<Velocity> fv;
        unsigned length;
    } dp;

    world->update<Position, Velocity>(dp.length, dp.fp, dp.fv);
    for(unsigned i = 0; i < dp.length; ++i) {
		//PlayerInput &pi = d.fpi.index(i);
        Position &position = dp.fp.index(i);
        //Velocity &velocity = d.fv.index(i);
        //Direction &direction = d.fd.index(i);
        //Engine::logn("Entity: %d", d.entities.index(i));
        Engine::logn("Position: %f", position.value.x);
    }

    struct TestSystemCData : ComponentData<PlayerInput, Position, Velocity, Direction> {
        ComponentArray<PlayerInput> input;
        ComponentArray<Position> position;
    };

    struct TestSystemECData : EntityComponentData<PlayerInput, Position, Velocity, Direction> {
        ComponentArray<PlayerInput> input;
        ComponentArray<Position> position;
    };

    struct TestSystemPData : ComponentData<Position, Velocity> {
        ComponentArray<Position> position;
        ComponentArray<Velocity> velocity;
    };

    TestSystemCData data;
    Engine::logn("\n TestSystemCData update");
    world->update_data(data, data.input, data.position);
    Engine::logn("TestSystemCData: %d", data.length);
    for(unsigned i = 0; i < data.length; ++i) {
        Engine::logn("PlayerInput: %f", data.input.index(i).move_x);
        Engine::logn("Position: %f", data.position.index(i).value.x);
    }

    TestSystemECData datadata;
    Engine::logn("\n TestSystemECData update");
    world->update_entity_data(datadata, datadata.entities, datadata.input, datadata.position);
    Engine::logn("TestSystemECData: %d", datadata.length);
    for(unsigned i = 0; i < datadata.length; ++i) {
        Engine::logn("PlayerInput: %f", datadata.input.index(i).move_x);
        Engine::logn("Position: %f", datadata.position.index(i).value.x);
        Engine::logn("Entity: %d", datadata.entities.index(i).id);
    }

    TestSystemPData pdata;
    Engine::logn("\n TestSystemPData update");
    world->update_data(pdata, pdata.position, pdata.velocity);
    Engine::logn("TestSystemPData: %d", pdata.length);
    for(unsigned i = 0; i < pdata.length; ++i) {
        Engine::logn("Position: %f", pdata.position.index(i).value.x);
        Engine::logn("Velocity: %f", pdata.velocity.index(i).value.x);
    }

    struct TestSystemEPData : EntityComponentData<Position, Velocity> {
        ComponentArray<Position> position;
        ComponentArray<Velocity> velocity;
    };

    TestSystemEPData epdata;
    world->update_entity_data(epdata, epdata.entities, epdata.position, epdata.velocity);
    Engine::logn("\n TestSystemEPData: %d", epdata.length);
    for(unsigned i = 0; i < epdata.length; ++i) {
        Engine::logn("Position: %f", epdata.position.index(i).value.x);
        Engine::logn("Velocity: %f", epdata.velocity.index(i).value.x);
        const Entity &ent = epdata.entities.index(i);
        Engine::logn("Entity | id: %d \t mask: %s \t generation: %d \t generation_index: %d", ent.id, ent.mask.to_string().c_str(), ent.generation(), ent.generation_index());
    }
}