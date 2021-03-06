#include "MainScene.h"
#include "PhysicsManager.h"


USING_NS_CC;

const int markRadias = 20;

Scene* MainScene::createScene()
{

    auto scene = Scene::create();
    auto layer = MainScene::create();
	auto menuLayer = MenuLayer::create();
	menuLayer->setMainScene(layer);
	scene->addChild(menuLayer,100);
    scene->addChild(layer);
    return scene;
}

// on "init" you need to initialize your instance
bool MainScene::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !Layer::init() )
    {
        return false;
    }
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();



	this->scheduleUpdate();

	auto listener = EventListenerTouchOneByOne::create();
	listener->onTouchBegan = CC_CALLBACK_2(MainScene::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(MainScene::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(MainScene::onTouchEnded, this);

	_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);


    return true;
}


void MainScene::menuCloseCallback(Ref* pSender)
{
#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
    return;
#endif

    Director::getInstance()->end();

#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
    exit(0);
#endif
}

void MainScene::menuChangeTouchModeCallback(Ref* pSender)
{
	if(PhysicsManager::getInstance()->getTouchType() == PhysicsManager::MOVE_TYPE)
		PhysicsManager::getInstance()->setTouchType(PhysicsManager::ADD_TYPE);
	else
			PhysicsManager::getInstance()->setTouchType(PhysicsManager::MOVE_TYPE);
}

void MainScene::draw(Renderer *renderer, const Mat4 &transform, uint32_t flags)
{
    Layer::draw(renderer, transform, flags);

    _customCmd.init(_globalZOrder);
    _customCmd.func = CC_CALLBACK_0(MainScene::onDraw, this, transform, flags);
    renderer->addCommand(&_customCmd);
}

void MainScene::onDraw(const Mat4 &transform, uint32_t flags)
{
    Director* director = Director::getInstance();
    CCASSERT(nullptr != director, "Director is null when seting matrix stack");
    director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
    director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, transform);

    GL::enableVertexAttribs( cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION );
	PhysicsManager::getInstance()->getWorld()->DrawDebugData();
    CHECK_GL_ERROR_DEBUG();
    
    director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}

void MainScene::update(float dt)
{
	PhysicsManager::getInstance()->update(dt);
}

bool MainScene::doMark(const Vec2& pos)
{
	_movingMark = getMark(pos);
	if(_movingMark)
	{
		if(_isDelete)
		{
			_marks.eraseObject(_movingMark);
			_movingMark->removeFromParent();
			_movingMark=nullptr;
			return false;
		}
		return true;
	}
	else
	{
		if(!_isDelete)
			addMark(pos);
		return false;
	}
}
bool MainScene::onTouchBegan(Touch* touch, Event* event)
{
	if(PhysicsManager::getInstance()->getTouchType() == PhysicsManager::ADD_TYPE)
		return true;
	if(PhysicsManager::getInstance()->getTouchType() == PhysicsManager::ADD_CUSTOM_TYPE)
	{
		auto pos = this->convertToNodeSpace(touch->getLocation());
		return doMark(pos);
	}
	if(PhysicsManager::getInstance()->getTouchType() == PhysicsManager::MOVE_TYPE)
	{
		auto touchLocation = touch->getLocation();    
		auto nodePosition = convertToNodeSpace( touchLocation );
//		log("MainScene::onTouchBegan, pos: %f,%f -> %f,%f", touchLocation.x, touchLocation.y, nodePosition.x, nodePosition.y);
		struct timeval now;
		gettimeofday(&now, NULL);
		auto nowTime = now.tv_sec*1000 + now.tv_usec/1000;
		if(nowTime - _preTime < 200){
			NotificationCenter::getInstance()->postNotification("toggleMenu", nullptr);
			log("Message: toggleMenu sent.");
		}
		_preTime = nowTime;
//		log("sec: %ld, usec: %ld", now.tv_sec, now.tv_usec);
		return PhysicsManager::getInstance()->MouseDown(nodePosition);
	}

	if(PhysicsManager::getInstance()->getTouchType() == PhysicsManager::ADD_JOINT_TYPE)
	{
		auto touchLocation = touch->getLocation();    
		auto nodePosition = convertToNodeSpace( touchLocation );
		//if(PhysicsManager::getInstance()->getJointType() == b2JointType::e_wheelJoint)
		//{
		//	return doMark(nodePosition);
		//}
		//if(PhysicsManager::getInstance()->getJointType() == b2JointType::e_distanceJoint)
		//{
		//	return doMark(nodePosition);
		//}
		//if(PhysicsManager::getInstance()->getJointType() == b2JointType::e_revoluteJoint)
		//{
		//	return doMark(nodePosition);
		//}
		return doMark(nodePosition);
		return false;
	}

	if(PhysicsManager::getInstance()->getTouchType() == PhysicsManager::SET_GRAVITY_TYPE)
	{
		//auto touchLocation = touch->getLocation();    
		//auto nodePosition = convertToNodeSpace( touchLocation );
		return true;
	}

	if(PhysicsManager::getInstance()->getTouchType() == PhysicsManager::DELETE_TYPE)
	{
		auto touchLocation = touch->getLocation();    
		auto nodePosition = convertToNodeSpace( touchLocation );
		PhysicsManager::getInstance()->deleteBodyAt(nodePosition);
		return false;
	}
	if(PhysicsManager::getInstance()->getTouchType() == PhysicsManager::NO_COLLIDE_TYPE 
		|| PhysicsManager::getInstance()->getTouchType() == PhysicsManager::COLLIDE_TYPE)
	{
		auto pos = this->convertToNodeSpace(touch->getLocation());
		return doMark(pos);
	}

	if(PhysicsManager::getInstance()->getTouchType() == PhysicsManager::ADD_GADGET)
	{
		auto touchLocation = touch->getLocation();    
		auto nodePosition = convertToNodeSpace( touchLocation );
		PhysicsManager::getInstance()->addGadgetAt(nodePosition);
		return false;
	}
	return false;
}

void MainScene::onTouchEnded(Touch* touch, Event* event)
{
	if(_movingMark)
		_movingMark = nullptr;
	if(PhysicsManager::getInstance()->getTouchType() == PhysicsManager::ADD_TYPE)
	{
		auto pos = this->convertToNodeSpace(touch->getLocation());
		PhysicsManager::getInstance()->addRegularPolygon(pos);
	}

	if(PhysicsManager::getInstance()->getTouchType() == PhysicsManager::ADD_CUSTOM_TYPE)
	{
		auto pos = this->convertToNodeSpace(touch->getLocation());
		
	}

	if(PhysicsManager::getInstance()->getTouchType() == PhysicsManager::MOVE_TYPE)
	{
		auto touchLocation = touch->getLocation();    
		auto nodePosition = convertToNodeSpace( touchLocation );
    
//		log("MainScene::onTouchEnded, pos: %f,%f -> %f,%f", touchLocation.x, touchLocation.y, nodePosition.x, nodePosition.y);
		PhysicsManager::getInstance()->MouseUp(nodePosition);
//		MouseUp(b2Vec2(nodePosition.x/PTM_RATIO,nodePosition.y/PTM_RATIO));
	}
	if(PhysicsManager::getInstance()->getTouchType() == PhysicsManager::SET_GRAVITY_TYPE)
	{
		//auto touchLocation = touch->getLocation();    
		//auto nodePosition = convertToNodeSpace( touchLocation );
		auto gravity = (touch->getLocation() - touch->getStartLocation())/10;
		PhysicsManager::getInstance()->setGravity(gravity);
		log("gravity: %f, %f", gravity.x, gravity.y);
//		PhysicsManager::getInstance()->setTouchType(PhysicsManager::MOVE_TYPE);
	}

}

void MainScene::onTouchMoved(Touch* touch, Event* event)
{
	if(PhysicsManager::getInstance()->getTouchType() == PhysicsManager::MOVE_TYPE)
	{
		if(PhysicsManager::getInstance()->isMovingBody())
		{
			auto touchLocation = touch->getLocation();    
			auto nodePosition = convertToNodeSpace( touchLocation );
    
//			log("MainScene::onTouchMoved, pos: %f,%f -> %f,%f", touchLocation.x, touchLocation.y, nodePosition.x, nodePosition.y);
			PhysicsManager::getInstance()->MouseMove(nodePosition);
//			MouseMove(b2Vec2(nodePosition.x/PTM_RATIO,nodePosition.y/PTM_RATIO));
		}
		else
		{
			auto delta = touch->getDelta();
			auto curPos = this->getPosition();
			this->setPosition(curPos + delta);
		}

	}
	//if(PhysicsManager::getInstance()->getTouchType() == PhysicsManager::ADD_CUSTOM_TYPE)
	//{
//		auto pos = this->convertToNodeSpace(touch->getLocation());
		if(_movingMark)
		{
			auto curPos = _movingMark->getPosition();
			_movingMark->setPosition(curPos + touch->getDelta());
		}
	//}

}

void MainScene::onExit()
{
	Layer::onExit();
	PhysicsManager::purgeInstance();
}

void MainScene::addMark(const Vec2& pos)
{
	if(_marks.size() >= _maxMark)
		return;
	MarkNode* draw = MarkNode::create();
	draw->drawDot(Vec2(0, 0), markRadias, Color4F(0, 1, 0, 1));
	draw->drawDot(Vec2(0, 0), 5, Color4F(1, 1, 1, 1));
	draw->setPosition(pos);
	this->addChild(draw);
	draw->setNum(_marks.size());
	_marks.pushBack(draw);

}

MarkNode* MainScene::getMark(const Vec2& pos)
{
	ssize_t size = _marks.size();
	for(ssize_t i=0; i<size; i++)
	{
		auto mPos = _marks.at(i)->getPosition();
		if((mPos - pos).length()<= markRadias * 2) // make marks easy to catch.
			return _marks.at(i);
	}
	return nullptr;
}

void MainScene::addCustomPolygon()
{
	if(_marks.size()<3)
		return;
	std::vector<Vec2> points;
	ssize_t size = _marks.size();
	for(ssize_t i=0; i<size; i++)
	{
		points.push_back(_marks.at(i)->getPosition());
	}
	PhysicsManager::getInstance()->addCustomPolygon(points);
}

void MainScene::addJoint()
{
	auto jointType = PhysicsManager::getInstance()->getJointType();
	if(jointType == b2JointType::e_distanceJoint || jointType ==b2JointType::e_wheelJoint
		|| jointType ==b2JointType::e_ropeJoint || jointType ==b2JointType::e_frictionJoint
		|| jointType ==b2JointType::e_motorJoint || jointType ==b2JointType::e_gearJoint)
	{
		if(_marks.size()<2)
			return;
		PhysicsManager::getInstance()->addJoint(_marks.at(0)->getPosition(),_marks.at(1)->getPosition());
	}
	if(jointType == b2JointType::e_revoluteJoint  || jointType == b2JointType::e_weldJoint )
	{
		if(_marks.size()<3)
			return;
		PhysicsManager::getInstance()->addJoint(_marks.at(0)->getPosition(),_marks.at(1)->getPosition(),_marks.at(2)->getPosition());
	}
	if(jointType == b2JointType::e_pulleyJoint || jointType == b2JointType::e_prismaticJoint)
	{
		if(_marks.size()<4)
			return;
		PhysicsManager::getInstance()->addJoint(_marks.at(0)->getPosition(),_marks.at(1)->getPosition(),
												_marks.at(2)->getPosition(),_marks.at(3)->getPosition());
	}

}

void MainScene::addNoCollide()
{
	auto type = PhysicsManager::getInstance()->getTouchType();
	if(type == PhysicsManager::NO_COLLIDE_TYPE || type == PhysicsManager::COLLIDE_TYPE)
	{
		if(_marks.size()<2)
			return;
		PhysicsManager::getInstance()->addNoCollide(_marks.at(0)->getPosition(),_marks.at(1)->getPosition());
	}
}

void MainScene::clearMarks()
{
	for(auto mk: _marks)
	{
		mk->removeFromParent();
	}
	_marks.clear();
}