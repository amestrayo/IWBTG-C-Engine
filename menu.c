void menuInit(Menu* m, float x, float y)
{
    m->itemCount = 0;
    m->firstItem = m->selected = 0;
    m->position.x = x;
    m->position.y = y;
    m->spacing.x = m->spacing.y = 10;
}

MenuItem* menuAddItem(Menu* m, MenuItemType type, char* label, Iwbtg* iw)
{
    // Fetch the first non-used menu item
    if(m->itemCount == MAX_ELEMENTS_PER_MENU)
    {
        printf("Error: No more room for menu items!\n");
        return 0;
    }
    MenuItem* e = &m->items[m->itemCount++];
    
    // Setup the menu item
    e->size = getTextSize(&iw->game.font, label);
    e->type = type;
    e->next = e->previous = 0;
    e->function = 0;
    e->functionData = 0;
    if(strlen(label) < MAX_LABEL_LENGTH)
        strcpy(e->label, label);
    else
        printf("Error: Menu item label too long!\n");
    
    // Add the item to the menu
    if(m->firstItem == 0)
    {
        m->firstItem = e;
        m->selected = e;
    }
    else
    {
        MenuItem* i = m->firstItem;
        while(i != 0)
        {
            if(i->next == 0)
            {
                i->next = e;
                e->previous = i;
                break;
            }
            i = i->next;
        }
    }
    
    m->lastItem = e;
    
    return e;
}

void updateMenu(Menu* m, Iwbtg* iw, float delta)
{
    Game* g = &iw->game;
    
    if(m->selected)
    {
        if(checkKeyPressed(g, KEY_DOWN))
        {
            if(m->selected->next)
                m->selected = m->selected->next;
            else
                m->selected = m->firstItem;
        }
        
        if(checkKeyPressed(g, KEY_UP))
        {
            if(m->selected->previous)
                m->selected = m->selected->previous;
            else
                m->selected = m->lastItem;
        }
        
        if(checkKeyPressed(g, KEY_JUMP))
        {
            if(m->selected && m->selected->function)
                m->selected->function(m->selected->functionData);
        }
    }
}

void drawMenu(Menu* m, Iwbtg* iw)
{
    // Draw each menu item
    Vector2f offset = m->position;
    for(MenuItem* e = m->firstItem; e != 0; e = e->next)
    {
        if(m->selected == e)
            drawTextCentered(&iw->game, 0, e->label, offset.x + 32, offset.y);
        else
            drawTextCentered(&iw->game, 0, e->label, offset.x, offset.y);
        offset.y += e->size.y + m->spacing.y;
    }
}