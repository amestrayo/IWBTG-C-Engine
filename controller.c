void entitySetControllerFromTypeIndex(Entity* e, int typeIndex)
{
    e->controller = &e->controllerData;
    
    if(typeIndex >= 0 && typeIndex <= 3) // Fast trap
    {
        e->controller->type = ControllerType_trap;
        e->controller->trap.direction = PI * 0.5f * typeIndex;
        e->controller->trap.activationDistance = 400;
        e->controller->trap.activationWidth = 32;
        e->controller->trap.speed = 32;
        e->controller->trap.activated = 0;
    }
    else if(typeIndex >= 4 && typeIndex <= 7) // Medium trap
    {
        e->controller->type = ControllerType_trap;
        e->controller->trap.direction = PI * 0.5f * (typeIndex - 4);
        e->controller->trap.activationDistance = 400;
        e->controller->trap.activationWidth = 32;
        e->controller->trap.speed = 16;
        e->controller->trap.activated = 0;
    }
    else if(typeIndex >= 8 && typeIndex <= 11) // Medium trap
    {
        e->controller->type = ControllerType_trap;
        e->controller->trap.direction = PI * 0.5f * (typeIndex - 8);
        e->controller->trap.activationDistance = 400;
        e->controller->trap.activationWidth = 32;
        e->controller->trap.speed = 4;
        e->controller->trap.activated = 0;
    }
    else if(typeIndex >= 12 && typeIndex <= 13) // In out
    {
        e->controller->type = ControllerType_inOut;
        e->controller->inOut.direction = PI * 0.5;
        e->controller->inOut.speed = 1;
        if(typeIndex == 13)
            e->controller->inOut.speed = 2;
        e->controller->inOut.distance = 32;
        e->controller->inOut.timer = 0;
        e->controller->inOut.basePosition = e->position;
    }
    else if(typeIndex == 16 || typeIndex == 17)
    {
        e->controller->type = ControllerType_bounce;
        if(typeIndex == 16)
            e->velocity.y = 1;
        else
            e->velocity.x = 1;
    }
    else if(typeIndex == 63) // Chaining controllers
    {
        e->controller->type = ControllerType_chain;
    }
}

void controllerUpdate(Controller* c, Entity* e, Iwbtg* iw, float dt)
{
    switch(c->type)
    {
        case ControllerType_trap: {
        
            ControllerTrap* trap = &e->controller->trap;
            
            
            float dx = cosf(trap->direction);
            float dy = -sinf(trap->direction);
            
            // Only check for activation on the owner of the controller
            if(c == &e->controllerData && !trap->activated)
            {
                Vector2f pos = vector2fAdd(e->position, vector2fDivide(vector2iTof(e->sprite.size), 2));
                Vector2f playerPos = {
                    iw->player.position.x + (iw->player.sprite.size.x / 2),
                    iw->player.position.y + (iw->player.sprite.size.y / 2)
                };
                
                Vector2f activationRange = {
                    dx * trap->activationDistance, 
                    dy * trap->activationDistance
                };
                
                Vector2f entityToPlayer = {
                    playerPos.x - pos.x,
                    playerPos.y - pos.y
                };
                
                Vector2f a = vector2fNormalize(activationRange);
                Vector2f b = vector2fDivide(entityToPlayer, trap->activationDistance);
                
                float dot = vector2fDot(a, b);
                Vector2f playerOnActivation = vector2fAdd(vector2fMultiply(activationRange, dot), pos);
                            
                if(dot >= 0 && dot <= 1 && vector2fDistance(playerOnActivation, playerPos) < (trap->activationWidth / 2))
                {
                    trap->activated = true;
                    soundPlay(assetsGetSound(&iw->game, "trap"), 1);
                    trap->activationFrame = iw->frameCount;
                }
            }

            if(trap->activated && (trap->activationFrame < iw->frameCount || !c->hasChains))
            {
                e->velocity.x = trap->speed * dx;
                e->velocity.y = trap->speed * dy;
            }
        } break;
        
        case ControllerType_inOut: {
            
            ControllerInOut* io = &e->controller->inOut;
            
            io->timer += dt * io->speed;
            float phase = fabs(sinf(io->timer));
            float dx = cosf(io->direction);
            float dy = -sinf(io->direction);
            
            e->position.x = io->basePosition.x + (dx * phase * io->distance) - (dx * io->distance);
            e->position.y = io->basePosition.y + (dy * phase * io->distance) - (dy * io->distance);
            
        } break;
        
        case ControllerType_bounce: {
            Rectanglef hitbox = { 0, 0, e->sprite.size.x, e->sprite.size.y };
            
            if(rectangleIsCollidingWithGround(&hitbox, iw, e->position.x, e->position.y))
                e->velocity = vector2fMultiply(e->velocity, -1);
        } break;
    }
}

void addChainLink(Level* l, unsigned char chainLinks[MAP_WIDTH][MAP_HEIGHT], int x, int y, Controller** joinedController)
{
    if(x > 0 && y > 0 && x < MAP_WIDTH && y < MAP_HEIGHT 
       && chainLinks[x][y] == 0)
    {
        int c = l->controllers.data[x + (y * l->entities.width)] - 1;

        if(c == 63) // chain link
        {
            chainLinks[x][y] = true;
            addChainLink(l, chainLinks, x-1, y, joinedController);
            addChainLink(l, chainLinks, x+1, y, joinedController);
            addChainLink(l, chainLinks, x, y-1, joinedController);
            addChainLink(l, chainLinks, x, y+1, joinedController);
        }
        else if(c != -1 && l->entityMap[x][y]) // controller is something other than chain link
            *joinedController = l->entityMap[x][y]->controller;
    }
}

void resolveChain(Entity* e, Iwbtg* iw)
{
    unsigned char chainLinks[MAP_WIDTH][MAP_HEIGHT];
    Controller* joinedController = 0;
    memset(chainLinks, 0, MAP_WIDTH * MAP_HEIGHT * sizeof(unsigned char));
    addChainLink(&iw->level, chainLinks, e->position.x / 32, e->position.y / 32, &joinedController);
    
    if(joinedController != 0)
    {
        joinedController->hasChains = true;
        for(int i = 0; i < MAP_WIDTH; ++i)
            for(int t = 0; t < MAP_HEIGHT; ++t)
            {
                if(chainLinks[i][t])
                {
                    Entity* e = iw->level.entityMap[i][t];
                    if(e)
                        e->controller = joinedController;
                }
            }
    }
}