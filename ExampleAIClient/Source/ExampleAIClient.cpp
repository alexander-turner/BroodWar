#include <BWAPI.h>
#include <BWAPI/Client.h>

#include <iostream>
#include <thread>
#include <chrono>
#include <string>

using namespace BWAPI;

void drawStats();
void drawBullets();
void drawVisibilityData();
void showPlayers();
void showForces();
bool show_bullets;
bool show_visibility_data;

void reconnect()
{
  while(!BWAPIClient.connect())
  {
    std::this_thread::sleep_for(std::chrono::milliseconds{ 1000 });
  }
}

int main(int argc, const char* argv[])
{
  std::cout << "Connecting..." << std::endl;;
  reconnect();
  while(true)
  {
    std::cout << "waiting to enter match" << std::endl;
    while ( !Broodwar->isInGame() )
    {
      BWAPI::BWAPIClient.update();
      if (!BWAPI::BWAPIClient.isConnected())
      {
        std::cout << "Reconnecting..." << std::endl;;
        reconnect();
      }
    }
    std::cout << "starting match!" << std::endl;
    Broodwar->sendText("Hello world!");
    Broodwar << "The map is " << Broodwar->mapName() << ", a " << Broodwar->getStartLocations().size() << " player map" << std::endl;
    // Enable some cheat flags
    Broodwar->enableFlag(Flag::UserInput);
    // Uncomment to enable complete map information
    Broodwar->enableFlag(Flag::CompleteMapInformation);

    show_bullets=false;
    show_visibility_data=false;

    if (Broodwar->isReplay())
    {
      Broodwar << "The following players are in this replay:" << std::endl;;
      Playerset players = Broodwar->getPlayers();
      for(auto p : players)
      {
        if ( !p->getUnits().empty() && !p->isNeutral() )
          Broodwar << p->getName() << ", playing as " << p->getRace() << std::endl;
      }
    }
    else
    {
      if (Broodwar->enemy())
        Broodwar << "The match up is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << std::endl;
      Unitset units    = Broodwar->self()->getUnits();
      for ( auto &u : units )
      {
<<<<<<< HEAD
		  /*std::vector<bool(*)(Unit, Unit)> actions;
=======
		  /*
		  std::vector<bool(*)(Unit, Unit)> actions;
>>>>>>> origin/master
		  Unitset targets = u->getUnitsInRadius(5);	//TODO: figure out optimal radius + enemies
		  Broodwar << targets << std::endl;

		  Unit target = targets.getClosestUnit(Broodwar->self()->getStartLocation());
		  bool(*attack_i)(Unit, Unit);
		  attack_i = &attackUnit;
		  actions.push_back(attack_i);

		  //Change return type of statefunction this in QFN.cpp or make a template
		  bool(*) startState;
		  startState = &u->isUnderAttack();

		  //Revise syntax
		  vector<vector<void(*)()>> features;
		  int(*) unitHP;
		  unitHP = &u->getHitPoints();
		  features.push_back(unitHP);

		  //Call Qfn
		  vector<vector<double>> QFunctionApproximation(startState, actions, features);*/

<<<<<<< HEAD
			Unitset enemies = Broodwar->enemy()->getUnits();
			/*std::vector<void(*)()> actions;
			// Enumerate available actions
			for (auto &enemy : enemies); {
				actions(1) = 
			}*/
			
			// Naive closest-attack
			//Unit firstEnemy = enemies.getClosestUnit();
			//u->attack(firstEnemy);
=======
>>>>>>> origin/master
      }
    }
    while(Broodwar->isInGame())
    {
		Broodwar->sendText("Test text");
		Unitset units = Broodwar->self()->getUnits();
		Unit u;
		u = Broodwar->getClosestUnit(Positions::Origin, Filter::IsOwned);
		Unitset enemies = Broodwar->enemy()->getUnits();
		Unit firstEnemy = enemies.getClosestUnit();
		for(auto &u: units)
		{
			u->attack(firstEnemy);
			Broodwar->sendText("[%s] attcking", u->getType().c_str());
			for (auto &e : enemies)
			{	if (firstEnemy->isUnderAttack() == NULL)
					Broodwar->sendText("Under attack returns null");
				else
					Broodwar->sendText("%s", firstEnemy->isUnderAttack());

				if (e->isUnderAttack())
				{
					Broodwar->sendText("[%s] under attack", e->getType().c_str());
					e->move(Positions::Origin);
					Broodwar->sendText("[%s] fleeing", e->getType().c_str());
				}

			}
		}


      for(auto &e : Broodwar->getEvents())
      {
        switch(e.getType())
        {
          case EventType::MatchEnd:
            if (e.isWinner())
              Broodwar << "I won the game" << std::endl;
            else
              Broodwar << "I lost the game" << std::endl;
            break;
          case EventType::SendText:
            if (e.getText()=="/show bullets")
            {
              show_bullets=!show_bullets;
            } else if (e.getText()=="/show players")
            {
              showPlayers();
            } else if (e.getText()=="/show forces")
            {
              showForces();
            } else if (e.getText()=="/show visibility")
            {
              show_visibility_data=!show_visibility_data;
            }
            else
            {
              Broodwar << "You typed \"" << e.getText() << "\"!" << std::endl;
            }
            break;
          case EventType::ReceiveText:
            Broodwar << e.getPlayer()->getName() << " said \"" << e.getText() << "\"" << std::endl;
            break;
          case EventType::PlayerLeft:
            Broodwar << e.getPlayer()->getName() << " left the game." << std::endl;
            break;
          case EventType::NukeDetect:
            if (e.getPosition()!=Positions::Unknown)
            {
              Broodwar->drawCircleMap(e.getPosition(), 40, Colors::Red, true);
              Broodwar << "Nuclear Launch Detected at " << e.getPosition() << std::endl;
            }
            else
              Broodwar << "Nuclear Launch Detected" << std::endl;
            break;
          case EventType::UnitCreate:
            if (!Broodwar->isReplay())
              Broodwar << "A " << e.getUnit()->getType() << " [" << e.getUnit() << "] has been created at " << e.getUnit()->getPosition() << std::endl;
            else
            {
              // if we are in a replay, then we will print out the build order
              // (just of the buildings, not the units).
              if (e.getUnit()->getType().isBuilding() && e.getUnit()->getPlayer()->isNeutral()==false)
              {
                int seconds=Broodwar->getFrameCount()/24;
                int minutes=seconds/60;
                seconds%=60;
                Broodwar->sendText("%.2d:%.2d: %s creates a %s", minutes, seconds, e.getUnit()->getPlayer()->getName().c_str(), e.getUnit()->getType().c_str());
              }
            }
            break;
          case EventType::UnitDestroy:
            if (!Broodwar->isReplay())
              Broodwar->sendText("A %s [%p] has been destroyed at (%d,%d)",e.getUnit()->getType().c_str(), e.getUnit(), e.getUnit()->getPosition().x, e.getUnit()->getPosition().y);
            break;
          case EventType::UnitMorph:
            if (!Broodwar->isReplay())
              Broodwar->sendText("A %s [%p] has been morphed at (%d,%d)",e.getUnit()->getType().c_str(), e.getUnit(), e.getUnit()->getPosition().x, e.getUnit()->getPosition().y);
            else
            {
              // if we are in a replay, then we will print out the build order
              // (just of the buildings, not the units).
              if (e.getUnit()->getType().isBuilding() && e.getUnit()->getPlayer()->isNeutral()==false)
              {
                int seconds=Broodwar->getFrameCount()/24;
                int minutes=seconds/60;
                seconds%=60;
                Broodwar->sendText("%.2d:%.2d: %s morphs a %s" ,minutes, seconds, e.getUnit()->getPlayer()->getName().c_str(), e.getUnit()->getType().c_str());
              }
            }
            break;
          case EventType::UnitShow:
            if (!Broodwar->isReplay())
              Broodwar->sendText("A %s [%p] has been spotted at (%d,%d)", e.getUnit()->getType().c_str(), e.getUnit(), e.getUnit()->getPosition().x, e.getUnit()->getPosition().y);
            break;
          case EventType::UnitHide:
            if (!Broodwar->isReplay())
              Broodwar->sendText("A %s [%p] was last seen at (%d,%d)", e.getUnit()->getType().c_str(), e.getUnit(), e.getUnit()->getPosition().x, e.getUnit()->getPosition().y);
            break;
          case EventType::UnitRenegade:
            if (!Broodwar->isReplay())
              Broodwar->sendText("A %s [%p] is now owned by %s", e.getUnit()->getType().c_str(), e.getUnit(), e.getUnit()->getPlayer()->getName().c_str());
            break;
          case EventType::SaveGame:
            Broodwar->sendText("The game was saved to \"%s\".", e.getText().c_str());
            break;
        }
      }

      if (show_bullets)
        drawBullets();

      if (show_visibility_data)
        drawVisibilityData();

      drawStats();
      Broodwar->drawTextScreen(300,0,"FPS: %f",Broodwar->getAverageFPS());

      BWAPI::BWAPIClient.update();
      if (!BWAPI::BWAPIClient.isConnected())
      {
        std::cout << "Reconnecting..." << std::endl;
        reconnect();
      }
    }
    std::cout << "Game ended" << std::endl;
  }
  std::cout << "Press ENTER to continue..." << std::endl;
  std::cin.ignore();
  return 0;
}



void drawStats()
{
  int line = 0;
  Broodwar->drawTextScreen(5, 0, "I have %d units:", Broodwar->self()->allUnitCount() );
  for (auto& unitType : UnitTypes::allUnitTypes())
  {
    int count = Broodwar->self()->allUnitCount(unitType);
    if ( count )
    {
      Broodwar->drawTextScreen(5, 16*line, "- %d %s%c", count, unitType.c_str(), count == 1 ? ' ' : 's');
      ++line;
    }
  }
}

void drawBullets()
{
  for (auto &b : Broodwar->getBullets())
  {
    Position p = b->getPosition();
    double velocityX = b->getVelocityX();
    double velocityY = b->getVelocityY();
    Broodwar->drawLineMap(p, p + Position((int)velocityX, (int)velocityY), b->getPlayer() == Broodwar->self() ? Colors::Green : Colors::Red);
    Broodwar->drawTextMap(p, "%c%s", b->getPlayer() == Broodwar->self() ? Text::Green : Text::Red, b->getType().c_str());
  }
}

void drawVisibilityData()
{
  int wid = Broodwar->mapHeight(), hgt = Broodwar->mapWidth();
  for ( int x = 0; x < wid; ++x )
    for ( int y = 0; y < hgt; ++y )
    {
      if ( Broodwar->isExplored(x, y) )
        Broodwar->drawDotMap(x*32+16, y*32+16, Broodwar->isVisible(x, y) ? Colors::Green : Colors::Blue);
      else
        Broodwar->drawDotMap(x*32+16, y*32+16, Colors::Red);
    }
}

void showPlayers()
{
  Playerset players = Broodwar->getPlayers();
  for(auto p : players)
    Broodwar << "Player [" << p->getID() << "]: " << p->getName() << " is in force: " << p->getForce()->getName() << std::endl;
}

void showForces()
{
  Forceset forces=Broodwar->getForces();
  for(auto f : forces)
  {
    Playerset players = f->getPlayers();
    Broodwar << "Force " << f->getName() << " has the following players:" << std::endl;
    for(auto p : players)
      Broodwar << "  - Player [" << p->getID() << "]: " << p->getName() << std::endl;
  }
}
