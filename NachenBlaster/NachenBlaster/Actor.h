#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

//TravelDir
const int NO_TRAVEL = -1;
const int UP_LEFT = 0;
const int DOWN_LEFT = 1;
const int DUE_LEFT = 2;

//Additional Tags
const int GAMEOBJECT = -1;
const int PLAYER_TORPEDO = 12;
const int ALIEN_TORPEDO = 13;
const int PLAYER = 14;

class StudentWorld;

class Actor:    public GraphObject
{
public:
    Actor(int imageID, double startX, double startY, int dir, double size, int depth);
    Actor(int imageID, double startX, double startY, StudentWorld* world, int dir, double size, int depth);
    virtual void doSomething() = 0;
    void die();
    bool isAlive() const;
    bool inBounds(double x, double y) const;
    StudentWorld* getWorld() const;
    bool collision(Actor* a2) const;
    virtual void act(int tag);
    virtual void sufferDamage(int enemy);
    virtual bool isCollidable(int enemy) const;
    int getTag() const;
    void setTag(int tag);
    bool isAlien(int tag) const;
private:
    bool alive;
    StudentWorld* m_world;
    int m_tag; //identifies each actor
};

//****** Star ******//
class Star:    public Actor
{
public:
    Star(double startX, double startY);
    virtual void doSomething();
};

//****** Explosion ******//
class Explosion:    public Actor
{
public:
    Explosion(double startX, double startY);
    virtual void doSomething();
private:
    int countdown;
};

//****** Damageable Object Abstract Class ******//
class Ship:    public Actor
{
public:
    Ship(int imageID, double startX, double startY, StudentWorld* world, double hitPts, int dir, double size, int depth);
    double getHitPts() const;
    void increaseHitPts(double amt);
    void decHitPts(double amt);
    virtual void fire(int tag) = 0;
private:
    double m_hitPts;
};

//****** NachenBlaster ******//
class NachenBlaster:    public Ship
{
public:
    NachenBlaster(StudentWorld* world);
    virtual void doSomething();
    virtual void sufferDamage(int enemy);
    virtual bool isCollidable(int enemy) const;
    virtual void fire(int tag);
    void incTorpedo(int torpedo);
    int getCabbages() const;
    int getTorpedoes() const;
private:
    int m_cabbage;
    int m_torpedo;
};

//****** Projectiles ******//
class Projectile:    public Actor
{
public:
    Projectile(int imageID, double startX, double startY, StudentWorld* world, int dir, double size, int depth);
    void actProj(int move, bool spin);
};

class Cabbage:    public Projectile
{
public:
    Cabbage(double startX, double startY, StudentWorld* world);
    virtual void doSomething();
};

class Turnip:   public Projectile
{
public:
    Turnip(double startX, double startY, StudentWorld* world);
    virtual void doSomething();
};

class Torpedo:  public Projectile
{
public:
    Torpedo(double startX, double startY, StudentWorld* world, int tag);
    virtual void doSomething();
};

//****** Aliens ******//
class Alien:    public Ship
{
public:
    Alien(int imageID, double startX, double startY, int levelNum, StudentWorld* world, int hits, double speed, int travelDir, int dir, double size, int depth);
    virtual bool isCollidable(int enemy) const;
    virtual void sufferDamage(int enemy);
    virtual void fire(int tag);
    virtual void act(int tag);
private:
    int m_flight; //flight plan length
    double m_speed;
    int m_travelDir;
};

class Smallgon:    public Alien
{
public:
    Smallgon(double startX, double startY, int levelNum, StudentWorld* world);
    virtual void doSomething();
};

class Smoregon:    public Alien
{
public:
    Smoregon(double startX, double startY, int levelNum, StudentWorld* world);
    virtual void doSomething();
};

class Snagglegon:   public Alien
{
public:
    Snagglegon(double startX, double startY, int levelNum, StudentWorld* world);
    virtual void doSomething();
};

//****** Goodies ******//
class Goodie:   public Actor
{
public:
    Goodie(int imageID, double startX, double startY, StudentWorld* world, int dir, double size, int depth);
    virtual void act(int tag);
    void activateMe(int good);
};

class Repair:   public Goodie
{
public:
    Repair(double startX, double startY, StudentWorld* world);
    virtual void doSomething();
};

class ExtraLife:    public Goodie
{
public:
    ExtraLife(double startX, double startY, StudentWorld* world);
    virtual void doSomething();
};

class TorpedoGoodie:    public Goodie
{
public:
    TorpedoGoodie(double startX, double startY, StudentWorld* world);
    virtual void doSomething();
};



#endif // ACTOR_H_
