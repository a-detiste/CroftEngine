#include "itemcontroller.h"

#include "animationcontroller.h"
#include "laracontroller.h"

namespace engine
{
    uint16_t ItemController::getCurrentAnimState() const
    {
        return m_dispatcher->getCurrentAnimState();
    }

    bool ItemController::handleTRTransitions()
    {
        return m_dispatcher->handleTRTransitions();
    }

    void ItemController::applyRotation()
    {
        //! @todo This is horribly inefficient code, but it properly converts ZXY angles to XYZ angles.
        irr::core::quaternion q;
        q.makeIdentity();
        q *= irr::core::quaternion().fromAngleAxis(getRotation().Y.toRad(), {0,1,0});
        q *= irr::core::quaternion().fromAngleAxis(getRotation().X.toRad(), {-1,0,0});
        q *= irr::core::quaternion().fromAngleAxis(getRotation().Z.toRad(), {0,0,-1});

        irr::core::vector3df euler;
        q.toEuler(euler);
        m_sceneNode->setRotation(euler * 180 / irr::core::PI);
    }

    void ItemController::setTargetState(uint16_t st)
    {
        Expects(m_dispatcher != nullptr);
        m_dispatcher->setTargetState(st);
    }

    uint16_t ItemController::getTargetState() const
    {
        Expects(m_dispatcher != nullptr);
        return m_dispatcher->getTargetState();
    }

    void ItemController::playAnimation(uint16_t anim, const boost::optional<irr::u32>& firstFrame)
    {
        Expects(m_dispatcher != nullptr);
        m_dispatcher->playLocalAnimation(anim, firstFrame);
    }

    void ItemController::nextFrame()
    {
        Expects(m_dispatcher != nullptr);
        m_dispatcher->advanceFrame();
    }

    irr::u32 ItemController::getCurrentFrame() const
    {
        Expects(m_dispatcher != nullptr);
        return m_dispatcher->getCurrentFrame();
    }

    irr::u32 ItemController::getAnimEndFrame() const
    {
        Expects(m_dispatcher != nullptr);
        return m_dispatcher->getAnimEndFrame();
    }

    ItemController::ItemController(const gsl::not_null<level::Level*>& level, const std::shared_ptr<engine::AnimationController>& dispatcher, const gsl::not_null<irr::scene::ISceneNode*>& sceneNode, const std::string & name, const gsl::not_null<const loader::Room*>& room, gsl::not_null<loader::Item*> item, bool hasProcessAnimCommandsOverride, uint8_t characteristics)
        : m_position(room)
        , m_level(level)
        , m_sceneNode(sceneNode)
        , m_dispatcher(dispatcher)
        , m_name(name)
        , m_hasProcessAnimCommandsOverride(hasProcessAnimCommandsOverride)
        , m_itemFlags(item->flags)
        , m_characteristics(characteristics)
    {
        auto nodeRot = sceneNode->getRotation();
        m_rotation.X = core::Angle::fromDegrees(nodeRot.X);
        m_rotation.Y = core::Angle::fromDegrees(nodeRot.Y);
        m_rotation.Z = core::Angle::fromDegrees(nodeRot.Z);

        m_sceneNode->updateAbsolutePosition();
        m_position.position = core::ExactTRCoordinates(m_sceneNode->getAbsolutePosition());

        setCurrentRoom(room);

        sceneNode->addAnimator(this);
        this->drop();

        if((m_itemFlags & Oneshot) != 0)
        {
            m_itemFlags &= ~Oneshot;
            m_flags2_02_toggledOn = true;
            m_flags2_04_ready = true;
        }

        if((m_itemFlags & ActivationMask) == ActivationMask)
        {
            m_itemFlags &= ~ActivationMask;
            m_itemFlags |= InvertedActivation;
            activate();
            m_flags2_02_toggledOn = true;
            m_flags2_04_ready = false;
        }
    }

    irr::core::aabbox3di ItemController::getBoundingBox() const
    {
        Expects(m_dispatcher != nullptr);
        return m_dispatcher->getBoundingBox();
    }

    void ItemController::setCurrentRoom(const loader::Room* newRoom)
    {
        if( newRoom == m_position.room )
            return;

        BOOST_LOG_TRIVIAL(debug) << "Room switch to " << newRoom->node->getName();
        if( newRoom == nullptr )
        {
            BOOST_LOG_TRIVIAL(fatal) << "No room to switch to. Matching rooms by position:";
            for( size_t i = 0; i < m_level->m_rooms.size(); ++i )
            {
                const loader::Room& room = m_level->m_rooms[i];
                if( room.node->getTransformedBoundingBox().isPointInside(m_sceneNode->getAbsolutePosition()) )
                {
                    BOOST_LOG_TRIVIAL(fatal) << "  - " << i;
                }
            }
            return;
        }

        m_position.room = newRoom;
        for( irr::u32 i = 0; i < m_sceneNode->getMaterialCount(); ++i )
        {
            irr::video::SMaterial& material = m_sceneNode->getMaterial(i);
            const auto col = m_position.room->lightColor.toSColor(1 - m_position.room->darkness / 8191.0f);
            material.EmissiveColor = col;
            material.AmbientColor = col;
        }
    }

    uint16_t ItemController::getCurrentAnimationId() const
    {
        Expects(m_dispatcher != nullptr);
        return m_dispatcher->getCurrentAnimationId();
    }

    float ItemController::calculateAnimFloorSpeed() const
    {
        Expects(m_dispatcher != nullptr);
        return m_dispatcher->calculateFloorSpeed();
    }

    int ItemController::getAnimAccelleration() const
    {
        Expects(m_dispatcher != nullptr);
        return m_dispatcher->getAccelleration();
    }

    void ItemController::processAnimCommands(bool advanceFrame)
    {
        m_flags2_10 = false;

        if(advanceFrame)
            nextFrame();

        bool newFrame = advanceFrame;

        if(handleTRTransitions() || m_lastAnimFrame != getCurrentFrame())
        {
            m_lastAnimFrame = getCurrentFrame();
            newFrame = true;
        }

        const bool isAnimEnd = getCurrentFrame() >= getAnimEndFrame();

        const loader::Animation& animation = getLevel().m_animations[getCurrentAnimationId()];
        if(animation.animCommandCount > 0)
        {
            BOOST_ASSERT(animation.animCommandIndex < getLevel().m_animCommands.size());
            const auto* cmd = &getLevel().m_animCommands[animation.animCommandIndex];
            for(uint16_t i = 0; i < animation.animCommandCount; ++i)
            {
                BOOST_ASSERT(cmd < &getLevel().m_animCommands.back());
                const auto opcode = static_cast<AnimCommandOpcode>(*cmd);
                ++cmd;
                switch(opcode)
                {
                    case AnimCommandOpcode::SetPosition:
                        if(isAnimEnd && newFrame)
                        {
                            moveLocal(
                                cmd[0],
                                cmd[1],
                                cmd[2]
                            );
                        }
                        cmd += 3;
                        break;
                    case AnimCommandOpcode::SetVelocity:
                        if(isAnimEnd && newFrame)
                        {
                            m_fallSpeed = cmd[0];
                            m_falling = true;
                            m_horizontalSpeed = cmd[1];
                        }
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::EmptyHands:
                        break;
                    case AnimCommandOpcode::PlaySound:
                        if(getCurrentFrame() == cmd[0])
                        {
                            //! @todo playsound(cmd[1])
                        }
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::PlayEffect:
                        if(getCurrentFrame() == cmd[0])
                        {
                            BOOST_LOG_TRIVIAL(debug) << "Anim effect: " << int(cmd[1]);
                            if(cmd[1] == 0 && newFrame)
                                addYRotation(180_deg);
                            else if(cmd[1] == 12)
                                getLevel().m_lara->setHandStatus(0);
                            //! @todo Execute anim effect cmd[1]
                        }
                        cmd += 2;
                        break;
                    case AnimCommandOpcode::Kill:
                        if(isAnimEnd && newFrame)
                        {
                            m_flags2_02_toggledOn = false;
                            m_flags2_04_ready = true;
                        }
                        break;
                    default:
                        break;
                }
            }
        }

        if(m_falling)
        {
            m_horizontalSpeed.add(getAnimAccelleration(), getCurrentDeltaTime());
            if(getFallSpeed() >= 128)
                m_fallSpeed.add(1, getCurrentDeltaTime());
            else
                m_fallSpeed.add(6, getCurrentDeltaTime());
        }
        else
        {
            m_horizontalSpeed = calculateAnimFloorSpeed();
        }

        move(
            getRotation().Y.sin() * m_horizontalSpeed.getScaled(getCurrentDeltaTime()),
            m_falling ? m_fallSpeed.getScaled(getCurrentDeltaTime()) : 0,
            getRotation().Y.cos() * m_horizontalSpeed.getScaled(getCurrentDeltaTime())
        );

        applyPosition();
    }

    void ItemController::activate()
    {
        if(!m_hasProcessAnimCommandsOverride)
        {
            m_flags2_02_toggledOn = false;
            m_flags2_04_ready = false;
            return;
        }

        if(m_isActive)
            BOOST_LOG_TRIVIAL(warning) << "Item controller " << m_name << " already active";
        else
            BOOST_LOG_TRIVIAL(trace) << "Activating item controller " << m_name;

        m_isActive = true;
    }

    void ItemController::deactivate()
    {
        if(!m_isActive)
            BOOST_LOG_TRIVIAL(warning) << "Item controller " << m_name << " already inactive";
        else
            BOOST_LOG_TRIVIAL(trace) << "Deactivating item controller " << m_name;

        m_isActive = false;
    }

    void ItemController_55_Switch::onInteract(LaraController& lara, LaraState& /*state*/)
    {
        if(!lara.getInputState().action)
            return;

        if(lara.getHandStatus() != 0)
            return;

        if(lara.isFalling())
            return;

        if(m_flags2_04_ready || m_flags2_02_toggledOn)
            return;

        if(lara.getCurrentState() != loader::LaraStateId::Stop)
            return;

        static const InteractionLimits limits{
            irr::core::aabbox3di{{-200, 0, 312}, {200, 0, 512}},
            {-10_deg,-30_deg,-10_deg},
            {+10_deg,+30_deg,+10_deg}
        };

        if(!limits.canInteract(*this, lara))
            return;

        lara.setYRotation(getRotation().Y);

        if(getCurrentAnimState() == 1)
        {
            BOOST_LOG_TRIVIAL(debug) << "Switch " << getName() << ": pull down";
            lara.setTargetState(loader::LaraStateId::SwitchDown);
            do
            {
                lara.processLaraAnimCommands(true);
            } while(lara.getCurrentAnimState() != loader::LaraStateId::SwitchDown);
            lara.setTargetState(loader::LaraStateId::Stop);
            setTargetState(0);
            lara.setHandStatus(1);
        }
        else
        {
            if(getCurrentAnimState() != 0)
                return;

            BOOST_LOG_TRIVIAL(debug) << "Switch " << getName() << ": pull up";
            lara.setTargetState(loader::LaraStateId::SwitchUp);
            do
            {
                lara.processLaraAnimCommands(true);
            } while(lara.getCurrentAnimState() != loader::LaraStateId::SwitchUp);
            lara.setTargetState(loader::LaraStateId::Stop);
            setTargetState(1);
            lara.setHandStatus(1);
        }

        m_flags2_04_ready = false;
        m_flags2_02_toggledOn = true;

        activate();
        ItemController::processAnimCommands();
    }

    bool InteractionLimits::canInteract(const ItemController& item, const LaraController& lara) const
    {
        const auto angle = lara.getRotation() - item.getRotation();
        if(   angle.X < minAngle.X || angle.X > maxAngle.X
           || angle.Y < minAngle.Y || angle.Y > maxAngle.Y
           || angle.Z < minAngle.Z || angle.Z > maxAngle.Z)
        {
            return false;
        }

        irr::core::quaternion q;
        q.makeIdentity();
        q *= irr::core::quaternion().fromAngleAxis(item.getRotation().Y.toRad(), { 0,1,0 });
        q *= irr::core::quaternion().fromAngleAxis(item.getRotation().X.toRad(), { -1,0,0 });
        q *= irr::core::quaternion().fromAngleAxis(item.getRotation().Z.toRad(), { 0,0,-1 });

        irr::core::matrix4 m = q.getMatrix();

        const auto dist = lara.getPosition() - item.getPosition();
        const auto dx = m(0, 0) * dist.X + m(0, 1) * dist.Y + m(0, 2) * dist.Z;
        const auto dy = m(1, 0) * dist.X + m(1, 1) * dist.Y + m(1, 2) * dist.Z;
        const auto dz = m(2, 0) * dist.X + m(2, 1) * dist.Y + m(2, 2) * dist.Z;

        return distance.isPointInside(irr::core::vector3di(dx, dy, dz));
    }

    void ItemController_Door::onInteract(LaraController& /*lara*/, LaraState& /*state*/)
    {
        
    }

}
