#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include <string>
#include <list>

double randDouble (double min, double max);

class Actor;
class NachenBlaster;

class StudentWorld : public GameWorld
{
public:
    StudentWorld(std::string assetDir);
    virtual ~StudentWorld();
    virtual int init();
    virtual int move();
    virtual void cleanUp();
    void removeDeadGameObjects();
    void updateDisplayText();
    bool processCollision(Actor* a2);
    void incDestroyedAliens();
    bool completedLevel() const;
    void addExplosion(double startX, double startY);
    void addGoodieMaybe(double startX, double startY, int tag);
    void addProjectile(double startX, double startY, int tag);
    NachenBlaster* getNB() const;

private:
    std::list<Actor*> m_actors;
    NachenBlaster* m_nb;
    int m_aliensDestroyed;
    int m_currAliens;
    bool canAddAlien() const;
    void addSomeAlien();
};

#endif // STUDENTWORLD_H_
