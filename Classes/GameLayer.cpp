#include "GameLayer.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;
using namespace CocosDenshion;

namespace 
{
	const float c_timeToSwitchDay = 20.f;
}

GameLayer::~GameLayer() 
{
    CC_SAFE_RELEASE(m_cyclistsMoving);
    CC_SAFE_RELEASE(m_cyclistsAnimation);
}

Scene* GameLayer::scene()
{
    auto scene = Scene::create();
    auto layer = GameLayer::create();

	scene->addChild(layer);
    return scene;
}

bool GameLayer::init()
{
    if (!Layer::init()) 
	{
        return false;
    }
    
    m_screenSize = Director::getInstance()->getWinSize();

    CreateGameScreen();
    
    ResetGame();
    
	InitListener();
	scheduleUpdate();
    
    return true;
}

void GameLayer::ResetGame() 
{
    m_score = 0;
    m_speedIncreaseInterval = 15;
	m_speedIncreaseTimer = 0;

	m_daySwitchTimer = 0;
	m_daySprite->setVisible(true);
	m_nightSprite->setVisible(false);
    m_scoreDisplay->setString(String::createWithFormat("%i", (int)m_score)->getCString());
    m_scoreDisplay->setAnchorPoint(Vec2(1,0));
    m_scoreDisplay->setPosition(Vec2(m_screenSize.width * 0.95f, m_screenSize.height * 0.88f));
	m_scoreDisplay->setVisible(false);
    m_gameState = GameState::Intro;
    
    m_introLabel->setVisible(true);
    m_mainMenu->setVisible(true);
    
    m_cyclists->setPosition(Vec2(m_screenSize.width * 0.19f, m_screenSize.height * 0.47f));
    m_cyclists->setVisible(true);
    m_cyclists->runAction(m_cyclistsAnimation);
    
    SimpleAudioEngine::getInstance()->stopBackgroundMusic();
    SimpleAudioEngine::getInstance()->playBackgroundMusic("background3.mp3", true);
    m_isRunning = true;
}

void GameLayer::GameOver()
{
	m_isRunning = false;
	m_gameState = GameState::Over;
	SetCatInvisible();
	SetSweeperInvisible();
	m_tryAgainLabel->setVisible(true);
	m_scoreDisplay->setAnchorPoint(Vec2(0.5f, 0.5f));
	m_scoreDisplay->setPosition(Vec2(m_screenSize.width * 0.5f, m_screenSize.height * 0.8f));

	m_playerHat->setPosition(Vec2(m_screenSize.width * 0.2f, -m_screenSize.height * 0.1f));
	m_playerHat->setVisible(true);
	m_playerHat->runAction(RotateBy::create(2.0f, 660));
	m_playerHat->runAction(JumpBy::create(2.0f, Vec2(0, 10), m_screenSize.height * 0.8f, 1));

	SimpleAudioEngine::getInstance()->stopBackgroundMusic();
	SimpleAudioEngine::getInstance()->playEffect("crashing.wav");
}

void GameLayer::CheckCollisions()
{
	m_area->CheckCollision(m_player);
	if (m_isCatFly)
	{
		if (m_player->getBoundingBox().intersectsRect(m_cat->getBoundingBox()))
		{
			if (!m_player->getFloating())
			{
				m_player->setState(PlayerState::PlayerDying);
			}
			else
			{
				m_cat->stopAllActions();
				m_cat->runAction(RotateBy::create(2.0f, 720.f));
				m_cat->runAction(m_catRush);
			}
		}
	}
	if (m_isSweeperCome)
	{
		if (m_player->getBoundingBox().intersectsRect(m_sweeper->getBoundingBox()))
		{
			m_player->setState(PlayerState::PlayerDying);
			m_sweeper->stopAllActions();
			m_sweeper->runAction(JumpBy::create(1.f, Vec2(0, 10), 50, 3));
			SetSweeperInvisible();
		}
	}
}

void GameLayer::UpdateSpritesPlacement()
{
	m_player->Move();
	if (m_isSweeperCome)
	{
		m_sweeper->setPositionX(m_sweeper->getPositionX() - m_player->getVelocity().x);
		if (m_sweepTimer > 0.5f)
		{
			m_sweeper->setPositionY(m_sweeperPosition);
		}
	}
	if (m_player->getExpectedPosition().y > m_screenSize.height * 0.6f)
	{
		m_mainBatchNode->setPositionY((m_screenSize.height * 0.6f - m_player->getExpectedPosition().y) * 0.8f);
		if (m_isSweeperCome && m_sweepTimer > 0.5f)
		{
			m_sweeper->setPositionY(m_sweeperPosition + m_mainBatchNode->getPositionY());
		}
	}
	else
	{
		m_mainBatchNode->setPositionY(0);
	}

	if (m_isCatFly)
	{
		m_cat->setPositionX(m_cat->getPositionX() - m_player->getVelocity().x);
		if (m_cat->getPosition().y < m_screenSize.height * 0.2)
		{
			SetCatInvisible();
		}
	}
}

void GameLayer::UpdateParallax()
{
	m_background->setPositionX(m_background->getPosition().x - m_player->getVelocity().x * 0.25f);
	float dx;

	if (m_background->getPositionX() < -m_background->getContentSize().width)
	{
		dx = abs(m_background->getPositionX()) - m_background->getContentSize().width;
		m_background->setPositionX(-dx);
	}

	m_foreground->setPositionX(m_foreground->getPosition().x - m_player->getVelocity().x * 4);

	if (m_foreground->getPositionX() < -m_foreground->getContentSize().width * 4)
	{
		dx = fabs(m_foreground->getPositionX()) - m_foreground->getContentSize().width * 4;
		m_foreground->setPositionX(-dx);
	}

	
	for (auto cloud : m_clouds)
	{
		
		cloud->setPositionX(cloud->getPositionX() - m_player->getVelocity().x * 0.8f);
		if (cloud->getPositionX() + cloud->getBoundingBox().size.width * 0.5f < 0)
		{
			cloud->setPositionX(m_screenSize.width + cloud->getBoundingBox().size.width * 0.5f);
		}
	}
}

void GameLayer::UpdateCyclists()
{
	if (m_cyclists->getPositionX() - m_player->getPosition().x > m_screenSize.width * 1.1f)
	{
		m_cyclists->stopAllActions();
		m_cyclists->setVisible(false);
	}
}

void GameLayer::UpdateScore(float dt)
{
	m_score += dt * 50;
	m_scoreDisplay->setString(String::createWithFormat("%i", (int)m_score)->getCString());
}

void GameLayer::IncreaseComplexity(float dt)
{
	m_speedIncreaseTimer += dt;
	if (m_speedIncreaseTimer > m_speedIncreaseInterval)
	{
		m_speedIncreaseTimer = 0;
		m_player->setMaxSpeed(m_player->getMaxSpeed() + 5);
	}
}

void GameLayer::PushCat()
{
	int chance = cocos2d::RandomHelper::random_int(0, 100);
	if (chance > 80)
	{
		auto chimsPos = m_area->GetChimneysPos();
		if (!chimsPos.empty())
		{	
			int randChim = cocos2d::RandomHelper::random_int(0, static_cast<int>(chimsPos.size() - 1));
			PushCatAtPosition(chimsPos.at(randChim));
			SimpleAudioEngine::getInstance()->playEffect("cat.wav");
			m_isCatFly = true;
		}
	}
}

void GameLayer::PushCatAtPosition(const cocos2d::Vec2 & position)
{
	m_cat->setVisible(true);
	m_cat->setPosition(position);
	m_cat->runAction(m_catRush);
}

void GameLayer::PushSweeper()
{
	int chance = cocos2d::RandomHelper::random_int(0, 100);
	if (chance > 80)
	{
		auto chimsPos = m_area->GetChimneysPos();
		if (!chimsPos.empty())
		{
			int randChim = cocos2d::RandomHelper::random_int(0, static_cast<int>(chimsPos.size() - 1));
			PushSweeperAtPosition(chimsPos.at(randChim));
			m_sweepTimer = 0;
			SimpleAudioEngine::getInstance()->playEffect("sneeze.wav");
			m_isSweeperCome = true;
		}
	}
}

void GameLayer::PushSweeperAtPosition(const cocos2d::Vec2 & position)
{
	m_sweeper->setVisible(true);
	m_sweeperPosition = position.y + m_sweeper->getBoundingBox().size.height / 2.f;
	m_sweeper->setPosition(position.x, m_sweeperPosition);
	m_sweeper->runAction(m_sweeperPush);
}

void GameLayer::SetCatInvisible()
{
	m_cat->setVisible(false);
	m_isCatFly = false;
}

void GameLayer::SetSweeperInvisible()
{
	m_sweeper->setVisible(false);
	m_isSweeperCome = false;
}

void GameLayer::UpdateTutorial()
{
	switch(m_gameState)
	{
	case GameState::TutorialJump:
		if (m_player->getState() == PlayerState::PlayerFalling && m_player->getVelocity().y < 0)
		{
			m_cyclists->stopAllActions();
			m_player->stopAllActions();
			m_isRunning = false;
			m_tutorialLabel->setString("Into the air, tap the screen to start floating!!!");
			m_gameState = GameState::TutorialFloat;
		}
		return;
	case GameState::TutorialFloat:
		if (m_player->getPositionY() < m_screenSize.height * 0.95f)
		{
			m_player->stopAllActions();
			m_cyclists->stopAllActions();
			m_isRunning = false;
			m_tutorialLabel->setString("While floating, tap on the screen again to drop.");
			m_gameState = GameState::TutorialDrop;
		}
		return;
	default:
		m_tutorialLabel->setString("That's it. Tap the screen to play.");
		m_gameState = GameState::Tutorial;
		return;
	}
}

void GameLayer::SwitchDay()
{
	if (m_nightSprite->isVisible())
	{
		m_nightSprite->setVisible(false);
		m_daySprite->setVisible(true);
	}
	else
	{
		m_nightSprite->setVisible(true);
		m_daySprite->setVisible(false);
	}
}

void GameLayer::update(float dt) 
{
	if (!m_isRunning)
	{ 
		return;
	}
    
    if (m_player->getPositionY() < -m_player->getHeight() || m_player->getPositionX() < -m_player->getWidth())
	{ 
        if (m_gameState == GameState::Play)
		{
			GameOver();
			return;
        }
    }
    
    m_player->update(dt);
    m_area->Move(m_player->getVelocity().x);
   
	if (m_player->getState() != PlayerState::PlayerDying) 
	{
		CheckCollisions();
		
	}
	UpdateSpritesPlacement();
    if (m_player->getVelocity().x > 0)
	{
		UpdateParallax();

		if (m_area->getStartGame())
		{
			UpdateScore(dt);
			IncreaseComplexity(dt);
		}
    }

    if (m_cyclists->isVisible())
	{
		UpdateCyclists();
    }

	if (m_gameState > GameState::Tutorial)
	{
		UpdateTutorial();
	}

	m_daySwitchTimer += dt;
	if (m_isSweeperCome)
	{
		m_sweepTimer += dt;
	}
	if (m_daySwitchTimer >= c_timeToSwitchDay)
	{
		m_daySwitchTimer = 0;
		SwitchDay();
	}
}

bool GameLayer::OnTouchBegan(Touch * touch, Event* event)
{
    if (touch)
	{
	    Point tap = touch->getLocation();
        
        switch (m_gameState)
		{
            case GameState::Intro:
                break;
            case GameState::Over:
                if (m_tryAgainLabel->getBoundingBox().containsPoint(tap)) 
				{
                    m_playerHat->setVisible(false);
                    m_playerHat->stopAllActions();
                    m_tryAgainLabel->setVisible(false);
                    m_area->Reset();
                    m_player->Reset();
                    
                    ResetGame();
                }
                break;
            case GameState::Play:
                if (m_player->getState() == PlayerState::PlayerFalling)
				{
					m_player->SetFloating(m_player->getFloating() ? false : true);
                }
				else 
				{
                    if (m_player->getState() != PlayerState::PlayerDying)
					{
                        SimpleAudioEngine::getInstance()->playEffect("jump.wav");
                        m_player->setJumping(true);
                    }
                } 
                m_area->ActivateChimneys(m_player);
				if (cocos2d::RandomHelper::random_int(0, 1) == 1 && !m_isCatFly)
				{
					PushCat();
				}
				else if(!m_isSweeperCome)
				{
					PushSweeper();
				}
                break;
            case GameState::Tutorial:
                m_tutorialLabel->setString("");
                m_tutorialLabel->setVisible(false);
                m_area->setStartGame(true);
                m_gameState = GameState::Play;
                break;
            case GameState::TutorialJump:
                if (m_player->getState() == PlayerState::PlayerMoving) 
				{
                    SimpleAudioEngine::getInstance()->playEffect("jump.wav");
                    m_player->setJumping(true);
                }
                break;
            case GameState::TutorialFloat:
                if (!m_player->getFloating())
				{
					m_cyclists->runAction(m_cyclistsMoving);
                    m_player->SetFloating(true);
                    m_isRunning = true;
                }
                break;
            case GameState::TutorialDrop:
				m_cyclists->runAction(m_cyclistsMoving);
                m_player->SetFloating(false);
                m_isRunning = true;
                break;
        }
    }

    return true;
}

void GameLayer::OnTouchEnded(Touch* touch, Event* event)
{
	if (m_gameState == GameState::Play)
	{
		m_player->setJumping(false);
	}
}

void GameLayer::ShowTutorial(Ref* pSender) 
{
    m_tutorialLabel->setString("Tap the screen to make the player jump.");
    m_gameState = GameState::TutorialJump;

    m_cyclists->runAction(m_cyclistsMoving);

	m_introLabel->setVisible(false);
    m_mainMenu->setVisible(false);

    SimpleAudioEngine::getInstance()->playEffect("start.wav");

    m_tutorialLabel->setVisible(true);
}

void GameLayer::StartGame(Ref* pSender)
{
    m_tutorialLabel->setVisible(false);
	m_introLabel->setVisible(false);
    m_mainMenu->setVisible(false);
	m_scoreDisplay->setVisible(true);
    
    m_cyclists->runAction(m_cyclistsMoving);

    SimpleAudioEngine::getInstance()->playEffect("start.wav");

    m_area->setStartGame(true);
    m_gameState = GameState::Play;
}

void GameLayer::CreateGameScreen()
{
	CreateFilling();
	CreateGameObjects();
	CreateMenu();
}

void GameLayer::CreateFilling()
{
	m_daySprite = Sprite::create("bg_day.png");
	m_daySprite->setPosition(Vec2(m_screenSize.width * 0.5f, m_screenSize.height * 0.5f));
	addChild(m_daySprite, LayerType::Back);

	m_nightSprite = Sprite::create("bg.png");
	m_nightSprite->setPosition(Vec2(m_screenSize.width * 0.5f, m_screenSize.height * 0.5f));
	addChild(m_nightSprite, LayerType::Back);

	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("sprite_sheet.plist");
	m_mainBatchNode = SpriteBatchNode::create("sprite_sheet.png", 200);
	addChild(m_mainBatchNode, LayerType::Middle);

	m_background = Sprite::createWithSpriteFrameName("background.png");
	m_background->setAnchorPoint(Vec2(0, 0));
	m_mainBatchNode->addChild(m_background, LayerType::Back);

	auto repeat = Sprite::createWithSpriteFrameName("background.png");
	repeat->setAnchorPoint(Vec2(0, 0));
	repeat->setPosition(Vec2(repeat->getContentSize().width - 1, 0));
	m_background->addChild(repeat, LayerType::Back);

	repeat = Sprite::createWithSpriteFrameName("background.png");
	repeat->setAnchorPoint(Vec2(0, 0));
	repeat->setPosition(Vec2(2 * (repeat->getContentSize().width - 1), 0));
	m_background->addChild(repeat, LayerType::Back);

	m_foreground = Sprite::createWithSpriteFrameName("lamp.png");
	m_foreground->setAnchorPoint(Vec2(0, 0));
	m_mainBatchNode->addChild(m_foreground, LayerType::Front + 1);

	repeat = Sprite::createWithSpriteFrameName("lamp.png");
	repeat->setAnchorPoint(Vec2(0, 0));
	repeat->setPosition(Vec2(repeat->getContentSize().width * 4, 0));
	m_foreground->addChild(repeat, LayerType::Back);

	repeat = Sprite::createWithSpriteFrameName("lamp.png");
	repeat->setAnchorPoint(Vec2(0, 0));
	repeat->setPosition(Vec2(repeat->getContentSize().width * 8, 0));
	m_foreground->addChild(repeat, LayerType::Back);

	float cloud_y;
	for (int i = 0; i < 4; i++)
	{
		cloud_y = i % 2 == 0 ? m_screenSize.height * 0.8f : m_screenSize.height * 0.9f;
		auto cloud = Sprite::createWithSpriteFrameName("cloud.png");
		cloud->setPosition(Vec2(m_screenSize.width * 0.15f + i * m_screenSize.width * 0.25f, cloud_y));
		m_mainBatchNode->addChild(cloud, LayerType::Back);
		m_clouds.pushBack(cloud);
	}

	m_tryAgainLabel = Sprite::createWithSpriteFrameName("label_tryagain.png");
	m_tryAgainLabel->setPosition(Vec2(m_screenSize.width * 0.5f, m_screenSize.height * 0.7f));
	m_tryAgainLabel->setVisible(false);
	addChild(m_tryAgainLabel, LayerType::Front);


	m_tutorialLabel = Label::createWithTTF("", "fonts/Times.ttf", 60);
	m_tutorialLabel->setPosition(Vec2(m_screenSize.width * 0.5f, m_screenSize.height * 0.6f));
	addChild(m_tutorialLabel, LayerType::Front);
	m_tutorialLabel->setVisible(false);

	cocos2d::TTFConfig config;
	config.fontSize = 200;
	config.fontFilePath = "fonts/Demo.ttf";
	m_introLabel = Label::createWithTTF(config, "AniSkyWorker`s: Rain Dogs", TextHAlignment::CENTER);
	m_introLabel->setPosition(Vec2(m_screenSize.width * 0.5f, m_screenSize.height * 0.7f));
	m_introLabel->setTextColor(cocos2d::Color4B::BLACK);
	addChild(m_introLabel, LayerType::Front);
}

void GameLayer::CreateGameObjects()
{
	m_area = Area::Create();
	m_mainBatchNode->addChild(m_area, LayerType::Front);

	m_player = Player::Create();
	m_mainBatchNode->addChild(m_player, LayerType::Back);

	m_scoreDisplay = Label::createWithBMFont("font.fnt", "000000", TextHAlignment::CENTER);
	m_scoreDisplay->setWidth(m_screenSize.width * 0.3f);
	m_scoreDisplay->setAnchorPoint(Vec2(1, 0));
	m_scoreDisplay->setPosition(Vec2(m_screenSize.width * 0.95f, m_screenSize.height * 0.88f));
	addChild(m_scoreDisplay, LayerType::Back);

	m_playerHat = Sprite::createWithSpriteFrameName("hat.png");
	m_playerHat->setVisible(false);
	m_mainBatchNode->addChild(m_playerHat, LayerType::Middle);

	m_cyclists = Sprite::createWithSpriteFrameName("jam_1.png");
	m_mainBatchNode->addChild(m_cyclists, LayerType::Back);
	InitCyclistsAnimation();
	m_cyclists->setPosition(Vec2(m_screenSize.width * 0.19f, m_screenSize.height * 0.47f));
	m_cyclistsMoving = MoveTo::create(3.0f, Vec2(m_screenSize.width * 1.5f, m_cyclists->getPositionY()));
	m_cyclistsMoving->retain();

	m_cat = Sprite::create("cat2.png");
	addChild(m_cat, LayerType::Middle);
	m_catRush = Sequence::create(JumpBy::create(2.0f, Vec2(0, 10), m_screenSize.height * 0.4f, 1)
		, CallFunc::create(std::bind(&GameLayer::SetCatInvisible, this)), nullptr);
	m_catRush->retain();
	m_cat->setVisible(false);

	m_sweeper = Sprite::create("sweeper.png");
	addChild(m_sweeper, LayerType::Middle);
	m_sweeperPush = Sequence::create(JumpBy::create(0.5f, Vec2(0,10), 10, 3), DelayTime::create(5.f), CallFunc::create(std::bind(&GameLayer::SetSweeperInvisible, this)), nullptr);
	m_sweeperPush->retain();
	m_sweeper->setVisible(false);
}

void GameLayer::CreateMenu()
{
	auto menuItemOn = Sprite::createWithSpriteFrameName("btn_new_on.png");
	auto menuItemOff = Sprite::createWithSpriteFrameName("btn_new_off.png");

	auto starGametItem = MenuItemSprite::create(menuItemOff, menuItemOn, CC_CALLBACK_1(GameLayer::StartGame, this));
	menuItemOn = Sprite::createWithSpriteFrameName("btn_howto_on.png");
	menuItemOff = Sprite::createWithSpriteFrameName("btn_howto_off.png");

	auto howToItem = MenuItemSprite::create(menuItemOff, menuItemOn, CC_CALLBACK_1(GameLayer::ShowTutorial, this));

	m_mainMenu = Menu::create(howToItem, starGametItem, nullptr);
	m_mainMenu->alignItemsHorizontallyWithPadding(120);
	m_mainMenu->setPosition(Vec2(m_screenSize.width * 0.5f, m_screenSize.height * 0.54));
	addChild(m_mainMenu, LayerType::Front);
}

void GameLayer::InitListener()
{
	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(GameLayer::OnTouchBegan, this);
	listener->onTouchEnded = CC_CALLBACK_2(GameLayer::OnTouchEnded, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);
}

void GameLayer::InitCyclistsAnimation()
{
	auto animation = Animation::create();
	for (int i = 1; i <= 3; i++)
	{
		auto frame = SpriteFrameCache::getInstance()->getSpriteFrameByName(String::createWithFormat("jam_%i.png", i)->getCString());
		animation->addSpriteFrame(frame);
	}

	animation->setDelayPerUnit(0.2f / 3.0f);
	animation->setRestoreOriginalFrame(false);
	animation->setLoops(-1);

	m_cyclistsAnimation = Animate::create(animation);
	m_cyclistsAnimation->retain();
}
