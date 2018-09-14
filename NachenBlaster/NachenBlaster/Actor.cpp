#include "Actor.h"
#include "StudentWorld.h"
#include "GameConstants.h"
#include <math.h>
#include <random>
using namespace std;

Actor::Actor(int imageID, double startX, double startY, int dir = 0, double size = 1.0, int depth = 0)
: GraphObject(imageID,startX,startY,dir,size,depth), alive(true), m_tag(GAMEOBJECT), m_world(nullptr)
{}

//Overloaded constructor includes StudentWorld pointer
Actor::Actor(int imageID, double startX, double startY, StudentWorld* world, int dir = 0, double size = 1.0, int depth = 0)
: GraphObject(imageID,startX,startY,dir,size,depth), alive(true), m_world(world), m_tag(GAMEOBJECT)
{}

bool Actor::isAlive() const { return alive; }
void Actor::die() { alive = false; }
StudentWorld* Actor::getWorld() const { return m_world; }

bool Actor::inBounds(double x, double y) const
{
    if (x < 0 || x >= VIEW_WIDTH || y < 0 || y >= VIEW_HEIGHT) //outside of screen
        return false;
    return true;
}

bool Actor::collision(Actor* a2) const //checks if this Actor and Actor a2 collided
{
    double x1 = getX();
    double y1 = getY();
    double r1 = getRadius();
    double x2 = a2->getX();
    double y2 = a2->getY();
    double r2 = a2->getRadius();
    
    double euclidian_dist = sqrt((x1-x2) * (x1-x2) + (y1-y2) * (y1-y2));
    if (euclidian_dist < .75 * (r1+r2))
        return true;
    return false;
}

//Each Actor has an identifier tag
//An Actor's tag is either its ID (from GameConstants.h) or from the additional tags declared in Actor.h
int Actor::getTag() const {return m_tag;}
void Actor::setTag(int tag){m_tag = tag;}

bool Actor::isAlien(int tag) const
{
    return (tag == IID_SMALLGON || tag == IID_SMOREGON|| tag == IID_SNAGGLEGON);
}

//Overridden by Alien and Goodie derived classes
void Actor::act(int tag) {}

//Overridden by NachenBlaster and aliens from the Alien class, which are able to take damage through collisions
void Actor::sufferDamage(int enemy) {}
bool Actor::isCollidable(int enemy) const {return false;}


//****** DERIVED CLASSES ******//

Star::Star(double startX, double startY)
: Actor(IID_STAR, startX, startY, 0, randDouble(.05, .5), 3) //randDouble is public function from StudentWorld.h/StudentWorld.cpp
{}

void Star::doSomething()
{
    double x = getX();
    double y = getY();
    if (inBounds(x-1, y))
        moveTo(x-1, y);
    else
        die();
}

Explosion::Explosion(double startX, double startY)
: Actor(IID_EXPLOSION, startX, startY), countdown(4)
{}

void Explosion::doSomething()
{
    setSize(getSize()*1.5);
    if (countdown == 1) //removes from game after 4 ticks
    {
        die();
        countdown = 4;
    }
    else
        countdown--;
}


//****** DAMAGEABLE OBJECT CLASS ******//

Ship::Ship(int imageID, double startX, double startY, StudentWorld* world, double hitPts, int dir = 0, double size = 1.0, int depth = 0)
: Actor(imageID, startX, startY, world, dir, size, depth), m_hitPts(hitPts)
{}

double Ship::getHitPts() const {return m_hitPts;}
void Ship::increaseHitPts(double amt) { //increases NachenBlaster's hit points
    if (m_hitPts + amt <= 50)
        m_hitPts += amt;
    else
        m_hitPts = 50; //if amt exceeds 50, set NachenBlaster's health to its max health (50 hit points)
}
void Ship::decHitPts(double amt) {m_hitPts -= amt;}


//****** PLAYER ******//

NachenBlaster::NachenBlaster(StudentWorld* world)
: Ship(IID_NACHENBLASTER, 0, 128, world, 50), m_cabbage(30), m_torpedo(0)
{setTag(PLAYER);}

void NachenBlaster::doSomething()
{
    if (!isAlive()) return;
    int ch;
    double x = getX();
    double y = getY();
    if (getWorld()->getKey(ch)) // user hit a key during this tick!
    {
        switch (ch)
        {
            case KEY_PRESS_LEFT:
                if (inBounds(x-6, y)) moveTo(x-6, y);
                break;
            case KEY_PRESS_RIGHT:
                if (inBounds(x+6, y)) moveTo(x+6, y);
                break;
            case KEY_PRESS_UP:
                if (inBounds(x, y+6)) moveTo(x, y+6);
                break;
            case KEY_PRESS_DOWN:
                if (inBounds(x, y-6)) moveTo(x, y-6);
                break;
            case KEY_PRESS_SPACE:
                fire(IID_CABBAGE);
                break;
            case KEY_PRESS_TAB:
                fire(PLAYER_TORPEDO);
                break;
        }
    }
    if (m_cabbage < 30)
        m_cabbage++;
}

void NachenBlaster::fire(int tag)
{
    if (tag == PLAYER_TORPEDO && m_torpedo > 0) //fire torpedo
    {
        getWorld()->addProjectile(getX()+12, getY(), PLAYER_TORPEDO);
        m_torpedo--;
        getWorld()->playSound(SOUND_TORPEDO);
        return;
    }
    else if (tag == IID_CABBAGE && m_cabbage >= 5) //fire cabbage
    {
        getWorld()->addProjectile(getX()+12, getY(), IID_CABBAGE);
        m_cabbage -= 5;
        getWorld()->playSound(SOUND_PLAYER_SHOOT);
    }
}

bool NachenBlaster::isCollidable(int enemy) const //player can only collide with an Alien or Alien projectile
{
    if (isAlien(enemy) || enemy == IID_TURNIP || enemy == ALIEN_TORPEDO)
        return true;
    return false;
}

void NachenBlaster::sufferDamage(int enemy)
{
    //player takes damage according to identity of the enemy
    if (enemy == IID_TURNIP)
        decHitPts(2);
    else if (enemy == IID_SNAGGLEGON)
        decHitPts(15);
    else if (isAlien(enemy))
        decHitPts(5);
    else if (enemy == ALIEN_TORPEDO)
        decHitPts(8);
    if (getHitPts() <= 0) //player loses a life
    {
        die();
        getWorld()->decLives();
    }
    else if (!isAlien(enemy)) //player hit by enemy projectile
        getWorld()->playSound(SOUND_BLAST);
}

void NachenBlaster::incTorpedo(int torpedo) { m_torpedo += torpedo;}
int NachenBlaster::getCabbages() const {return m_cabbage;}
int NachenBlaster::getTorpedoes() const {return m_torpedo;}


//****** PROJECTILES ******//

Projectile::Projectile(int imageID, double startX, double startY, StudentWorld* world, int dir = 0, double size = 0.5, int depth = 1)
: Actor(imageID, startX, startY, world, dir, size, depth)
{}

void Projectile::actProj(int move, bool spin)
{
    if (!isAlive()) return;
    double x = getX();
    double y = getY();
    
    if (x < 0 || x >= VIEW_WIDTH) {
        die();
        return;
    }
    
    if (getWorld()->processCollision(this)) { //check collision: triggers its target's sufferDamage function
        die();
        return;
    }
    
    if (spin) setDirection(getDirection()+20); //does this projectile spin?
    moveTo(x+move, y);
    
    if (getWorld()->processCollision(this)) //check collision again
        die();
}

Cabbage::Cabbage(double startX, double startY, StudentWorld* world)
: Projectile(IID_CABBAGE, startX, startY, world)
{setTag(IID_CABBAGE);}

void Cabbage::doSomething()
{
    actProj(8, true);
}

Turnip::Turnip(double startX, double startY, StudentWorld* world)
: Projectile(IID_TURNIP, startX, startY, world)
{setTag(IID_TURNIP);}

void Turnip::doSomething()
{
    actProj(-6, true);
}

Torpedo::Torpedo(double startX, double startY, StudentWorld* world, int tag)
: Projectile(IID_TORPEDO, startX, startY, world, (tag == PLAYER_TORPEDO) ? 0 : 180)
{setTag(tag);}

void Torpedo::doSomething()
{
    if (getTag() == PLAYER_TORPEDO) //player torpedo moves 8 px to left
        actProj(8, false);
    else
        actProj(-8, false); //alien torpedo moves 8 px to right
}


//****** ALIENS ******//

Alien::Alien(int imageID, double startX, double startY, int levelNum, StudentWorld* world, int hits = 5, double speed = 2.0, int travelDir = DOWN_LEFT, int dir = 0, double size = 1.5, int depth = 1)
: Ship(imageID, startX, startY, world, hits * (1 + (levelNum - 1) * .1), dir, size, depth), m_flight(0), m_speed(speed), m_travelDir(travelDir)
{}

bool Alien::isCollidable(int enemy) const //alien can only collide with player or player's projectiles
{
    if (enemy == PLAYER || enemy == IID_CABBAGE || enemy == PLAYER_TORPEDO)
        return true;
    return false;
}

void Alien::sufferDamage(int enemy)
{
    //alien takes damage according to identity of its enemy
    if (enemy == IID_CABBAGE)
        decHitPts(2);
    else if (enemy == PLAYER_TORPEDO)
        decHitPts(8);
    
    if (getHitPts() <= 0 || enemy == PLAYER) //alien is dead!
    {
        if (enemy == IID_SNAGGLEGON)
            getWorld()->increaseScore(1000);
        else
            getWorld()->increaseScore(250);
        die();
        getWorld()->incDestroyedAliens();
        getWorld()->playSound(SOUND_DEATH);
        getWorld()->addExplosion(getX(), getY());
        
        //Smoregons and Snagglegons might drop goodies
        if (getTag() == IID_SMOREGON)
            getWorld()->addGoodieMaybe(getX(), getY(), GAMEOBJECT);
        if (getTag() == IID_SNAGGLEGON)
            getWorld()->addGoodieMaybe(getX(), getY(), IID_LIFE_GOODIE);
    }
    else
        getWorld()->playSound(SOUND_BLAST);
}

void Alien::fire(int tag)
{
    if (tag == ALIEN_TORPEDO) { //fire torpedo and play its sound
        getWorld()->playSound(SOUND_TORPEDO);
        getWorld()->addProjectile(getX()-14, getY(), ALIEN_TORPEDO);
        return;
    }
    //else: fire a turnip
    getWorld()->playSound(SOUND_ALIEN_SHOOT);
    getWorld()->addProjectile(getX()-14, getY(), IID_TURNIP);
}

void Alien::act(int tag) 
{
    if (!isAlive()) return;
    double x = getX();
    double y = getY();
    
    if (x < 0) {
        die();
        return;
    }
    
    if (getWorld()->processCollision(this)) { //checks collision with player: triggers player's sufferDamage function
        sufferDamage(PLAYER); //then calls Alien's own sufferDamage function
        return;
    }
    
    if (m_flight == 0 || y >= VIEW_HEIGHT-1 || y <= 0) //if flight path reaches 0 or reaches bottom/top of screen
    {
        if (y >= VIEW_HEIGHT-1)
            m_travelDir = DOWN_LEFT;
        else if (y <= 0)
            m_travelDir = UP_LEFT;
        else if (m_flight == 0 && tag != IID_SNAGGLEGON)
            m_travelDir = randInt(0, 2); //random travel direction: down and left, up and left, or due left
        if (tag != IID_SNAGGLEGON) m_flight = randInt(1, 32);
    }
    
    //Potentially fire a projectile
    NachenBlaster* nb = getWorld()->getNB();
    const int rand = randInt(0, ((20/getWorld()->getLevel()+5)-1));
    const int randSmor = randInt(0, ((20/getWorld()->getLevel()+5)-1));
    const int randSnag = randInt(0, ((15/getWorld()->getLevel()+10)-1));
    if (nb->getX() < x && nb->getY() >= y-4 && nb->getY() <= y+4)
    {
        if (tag == IID_SNAGGLEGON && randSnag < 1) { //Snagglegon fires torpedo
            fire(ALIEN_TORPEDO);
            return;
        }
        else if (tag != IID_SNAGGLEGON && rand < 1) { //other aliens fire turnips
            fire(IID_TURNIP);
            return;
        }
        if (tag == IID_SMOREGON && randSmor < 1) //Smoregon randomly charges
        {
            m_travelDir = DUE_LEFT;
            m_flight = VIEW_WIDTH;
            m_speed = 5;
        }
    }
    
    switch (m_travelDir) { //move according to travelDir
        case UP_LEFT:
            moveTo(x - m_speed, y + m_speed);
            break;
        case DOWN_LEFT:
            moveTo(x - m_speed, y - m_speed);
            break;
        case DUE_LEFT:
            moveTo(x - m_speed, y);
            break;
    }
    m_flight--;
    
    if (getWorld()->processCollision(this)) { //checks collision again
        sufferDamage(PLAYER);
    }
}

Smallgon::Smallgon(double startX, double startY, int levelNum, StudentWorld* world)
: Alien(IID_SMALLGON, startX, startY, levelNum, world)
{setTag(IID_SMALLGON);}

void Smallgon::doSomething()
{
    act(IID_SMALLGON);
}

Smoregon::Smoregon(double startX, double startY, int levelNum, StudentWorld* world)
: Alien(IID_SMOREGON, startX, startY, levelNum, world)
{setTag(IID_SMOREGON);}

void Smoregon::doSomething()
{
    act(IID_SMOREGON);
}

Snagglegon::Snagglegon(double startX, double startY, int levelNum, StudentWorld* world)
: Alien(IID_SNAGGLEGON, startX, startY, levelNum, world, 10, 1.75)
{setTag(IID_SNAGGLEGON);}

void Snagglegon::doSomething()
{
    act(IID_SNAGGLEGON);
}


//****** GOODIES ******//
Goodie::Goodie(int imageID, double startX, double startY, StudentWorld* world, int dir = 0, double size = .5, int depth = 1)
: Actor(imageID, startX, startY, world, dir, size, depth)
{}

void Goodie::act(int tag)
{
    if (!isAlive()) return;
    double x = getX();
    double y = getY();
    
    if (!inBounds(x, y)){
        die();
        return;
    }

    if (getWorld()->getNB()->collision(this)) //check collision with player
    {
        activateMe(tag); //activates this goodie's benefits
        return;
    }

    moveTo(x-.75, y-.75);

    if (getWorld()->getNB()->collision(this))
        activateMe(tag);
}

void Goodie::activateMe(int good)
{
    StudentWorld* w = getWorld();
    w->increaseScore(100);
    die();
    w->playSound(SOUND_GOODIE);
    switch (good) { //gives different benefits to player depending on identity of goodie
        case IID_REPAIR_GOODIE:
            w->getNB()->increaseHitPts(10);
            break;
        case IID_LIFE_GOODIE:
            w->incLives();
            break;
        case IID_TORPEDO_GOODIE:
            w->getNB()->incTorpedo(5);
            break;
    }
}

Repair::Repair(double startX, double startY, StudentWorld* world)
: Goodie(IID_REPAIR_GOODIE, startX, startY, world)
{}

void Repair::doSomething()
{
    act(IID_REPAIR_GOODIE);
}

ExtraLife::ExtraLife(double startX, double startY, StudentWorld* world)
: Goodie(IID_LIFE_GOODIE, startX, startY, world)
{}

void ExtraLife::doSomething()
{
    act(IID_LIFE_GOODIE);
}

TorpedoGoodie::TorpedoGoodie(double startX, double startY, StudentWorld* world)
: Goodie(IID_TORPEDO_GOODIE, startX, startY, world)
{}

void TorpedoGoodie::doSomething()
{
    act(IID_TORPEDO_GOODIE);
}

