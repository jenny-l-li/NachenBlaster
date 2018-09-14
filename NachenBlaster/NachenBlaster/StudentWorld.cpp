#include "StudentWorld.h"
#include "GameConstants.h"
#include "Actor.h"
#include <math.h>
#include <random>
#include <sstream>
#include <iomanip>
using namespace std;

GameWorld* createStudentWorld(string assetDir)
{
	return new StudentWorld(assetDir);
}

double randDouble (double min, double max) //generate random double
{
    double r = (double)rand() / (double)RAND_MAX;
    return min + r * (max - min);
}

StudentWorld::StudentWorld(string assetDir)
: GameWorld(assetDir), m_aliensDestroyed(0), m_currAliens(0), m_nb(nullptr)
{}

StudentWorld::~StudentWorld()
{
    cleanUp();
}


int StudentWorld::init()
{
    m_currAliens = 0;
    m_aliensDestroyed = 0;
    //initialize stars: can use setSize here too 
    for (int k = 0; k < 30; k++)
        m_actors.push_back(new Star((double)randInt(0, VIEW_WIDTH-1),(double)randInt(0, VIEW_HEIGHT-1)));
    
    //initialize player
    m_nb = new NachenBlaster(this);
    
    return GWSTATUS_CONTINUE_GAME;
}

int StudentWorld::move()
{
    //go through NachenBlaster's doSomething
    m_nb->doSomething();
    if (!m_nb->isAlive())
        return GWSTATUS_PLAYER_DIED;
    if (completedLevel())
        return GWSTATUS_FINISHED_LEVEL;
    
    list<Actor*>::iterator itr;
    itr = m_actors.begin();
    while (itr != m_actors.end()) //go through doSomething for all the other actors
    {
        Actor* ap = *itr;
        ap->doSomething();
        if (!m_nb->isAlive())
            return GWSTATUS_PLAYER_DIED;
        if (completedLevel()) {
            playSound(SOUND_FINISHED_LEVEL);
            return GWSTATUS_FINISHED_LEVEL;
        }
        itr++;
    }
    removeDeadGameObjects();
    updateDisplayText();
    
    //Add a new star, potentially
    if (randInt(0, 14) < 1) //range 0 to 14
        m_actors.push_back(new Star(VIEW_WIDTH-1,randDouble(0, VIEW_HEIGHT-1)));
    
    //Add new alien
    if (canAddAlien())
        addSomeAlien();
    
    return GWSTATUS_CONTINUE_GAME;
}

bool StudentWorld::canAddAlien() const //checks if alien can be added
{
    int total =  6 + (4 * getLevel());
    int remaining = total - m_aliensDestroyed;
    double max = 4 + (.5 * getLevel());
    double min = (remaining < max - 1) ? remaining : max - 1;
    return (m_currAliens < min);
}

void StudentWorld::addSomeAlien()
{
    int s1 = 60;
    int s2 = 20 + getLevel() * 5;
    int s3 = 5 + getLevel() * 10;
    int s = s1 + s2 + s3;
    int r = randInt(0, s-1);
    if (r < s1) //s1/s chance
        m_actors.push_back(new Smallgon(VIEW_WIDTH-1,randDouble(0, VIEW_HEIGHT-1), getLevel(), this));
    else if (r >= s1 && r < s1+s2) //s2/s chance
        m_actors.push_back(new Smoregon(VIEW_WIDTH-1,randDouble(0, VIEW_HEIGHT-1), getLevel(), this));
    else if (r < s) //s3/s chance
        m_actors.push_back(new Snagglegon(VIEW_WIDTH-1,randDouble(0, VIEW_HEIGHT-1), getLevel(), this));
    m_currAliens++;
}

void StudentWorld::cleanUp()
{
    list<Actor*>::iterator itr;
    itr = m_actors.begin();
    while (itr != m_actors.end())
    {
        delete *itr;
        itr = m_actors.erase(itr);
    }
    if (m_nb != nullptr) //delete NachenBlaster
    {
        delete m_nb;
        m_nb = nullptr;
    }
}

void StudentWorld::removeDeadGameObjects()
{
    list<Actor*>::iterator itr;
    itr = m_actors.begin();
    while (itr != m_actors.end())
    {
        Actor* ap = *itr;
        if (!ap->isAlive())
        {
            if (ap->isAlien(ap->getTag())) //if a dead alien is removed, decrease current num of aliens
                m_currAliens--;
            delete *itr;
            itr = m_actors.erase(itr);
            continue;
        }
        itr++;
    }
}

void StudentWorld::updateDisplayText() //sets display text
{
    ostringstream oss; //setw attachs to the one after
    oss.setf(ios::fixed);
    oss.precision(0);
    oss << "Lives: " << getLives() << setw(10) << "Health: " << m_nb->getHitPts()/50.0 * 100 << "%" << setw(9) << "Score: " << getScore() << setw(9) << "Level: " << getLevel() << setw(12) << "Cabbages: " << m_nb->getCabbages()/30.0 * 100 << "%" << setw(13) << "Torpedoes: " << m_nb->getTorpedoes();
    setGameStatText(oss.str());
}

void StudentWorld::incDestroyedAliens()
{
    m_aliensDestroyed++;
}

bool StudentWorld::processCollision(Actor* a2) //process a collision from another actor
{
    if (m_nb->isCollidable(a2->getTag()) && m_nb->collision(a2)) //checks collision with a2
    {
        m_nb->sufferDamage(a2->getTag()); //player will suffer damage from a2
        return true;
    }
    list<Actor*>::iterator itr;
    itr = m_actors.begin();
    while (itr != m_actors.end())
    {
        Actor* ap = *itr;
        if (ap->isCollidable(a2->getTag()) && ap->collision(a2)) //checks collision with a2
        {
            ap->sufferDamage(a2->getTag()); //this actor will suffer damage from a2
            return true;
        }
        itr++;
    }
    return false;
}

bool StudentWorld::completedLevel() const
{
    return 6 + (4 * getLevel()) == m_aliensDestroyed;
}

NachenBlaster* StudentWorld::getNB() const {return m_nb;}

void StudentWorld::addExplosion(double startX, double startY)
{
    m_actors.push_back(new Explosion(startX,startY));
}

void StudentWorld::addGoodieMaybe(double startX, double startY, int tag) //add goodie based on tag
{
    const int rLife = randInt(0, 5); //1/6 chance
    if (tag == IID_LIFE_GOODIE) {
        if (rLife < 1)
            m_actors.push_back(new ExtraLife(startX, startY, this));
        return;
    }
    
    const int r = randInt(0, 2); //1/3 chance
    if (r < 1)
    {
        const int r2 = randInt(0, 1); //1/2 chance
        if (r2 < 1)
            m_actors.push_back(new Repair(startX, startY, this));
        else
            m_actors.push_back(new TorpedoGoodie(startX, startY, this));
    }
}

void StudentWorld::addProjectile(double startX, double startY, int tag) //add projectile based on tag
{
    switch (tag) {
        case IID_CABBAGE:
            m_actors.push_back(new Cabbage(startX, startY, this));
            break;
        case IID_TURNIP:
            m_actors.push_back(new Turnip(startX, startY, this));
            break;
        case PLAYER_TORPEDO:
            m_actors.push_back(new Torpedo(startX, startY, this, tag));
            break;
        case ALIEN_TORPEDO:
            m_actors.push_back(new Torpedo(startX, startY, this, tag));
            break;
    }
}
