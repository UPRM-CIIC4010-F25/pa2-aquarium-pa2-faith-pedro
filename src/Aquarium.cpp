#include "Aquarium.h"
#include <cstdlib>


string AquariumCreatureTypeToString(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return "BiggerFish";
        case AquariumCreatureType::NPCreature:
            return "BaseFish";
        case AquariumCreatureType::PescaoCute:
            return "PescaoCute";
        case AquariumCreatureType::ClownFish:
            return "ClownFish";
        default:
            return "UknownFish";
    }
}

// PlayerCreature Implementation
PlayerCreature::PlayerCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 10.0f, 1, sprite) {
    m_base_speed = speed;
}


void PlayerCreature::setDirection(float dx, float dy) {
    m_dx = dx;
    m_dy = dy;
    normalize();
}

void PlayerCreature::move() {
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    this->handleBounds();
}

void PlayerCreature::reduceDamageDebounce() {
    if (m_damage_debounce > 0) {
        --m_damage_debounce;
    }
}

void PlayerCreature::activatePowerUp() {
    if (!m_powered_up) {
        m_powered_up = true;
        m_speed = static_cast<int>(m_base_speed * 1.5f);
        m_powerup_timer = POWERUP_DURATION;
        ofLogNotice() << "Power-up activated. Speed increased: " << m_speed;
    }
}

void PlayerCreature::update() {
    this->reduceDamageDebounce();

    if (m_powered_up) {
        m_powerup_timer--;
        if (m_powerup_timer <= 0) {
            m_powered_up = false;
            m_speed = m_base_speed;
            ofLogNotice() << "Power-up expired. Speed: " << m_speed;
        }
    }

    this->move();
}


void PlayerCreature::draw() const {
    ofLogVerbose() << "PlayerCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    
    if (this->m_damage_debounce > 0) {
        ofSetColor(ofColor::red);
    } else if (m_powered_up) {
        ofSetColor(ofColor::yellow);
    }

    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
    ofSetColor(ofColor::white); // Reset color

}

void PlayerCreature::changeSpeed(int speed) {
    m_speed = speed;
}

void PlayerCreature::loseLife(int debounce) {
    if (m_damage_debounce <= 0) {
        if (m_lives > 0) this->m_lives -= 1;
        m_damage_debounce = debounce; // Set debounce frames
        ofLogNotice() << "Player lost a life! Lives remaining: " << m_lives << std::endl;
    }
    // If in debounce period, do nothing
    if (m_damage_debounce > 0) {
        ofLogVerbose() << "Player is in damage debounce period. Frames left: " << m_damage_debounce << std::endl;
    }
}

// NPCreature Implementation
NPCreature::NPCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 30, 1, sprite) {
    m_dx = (rand() % 3 - 1); // -1, 0, or 1
    m_dy = (rand() % 3 - 1); // -1, 0, or 1
    normalize();

    m_creatureType = AquariumCreatureType::NPCreature;
}

void NPCreature::move() {
    // Simple AI movement logic (random direction)
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }
    this->handleBounds(); //Mantiene dentro de los bordes
}

void NPCreature::draw() const {
    ofLogVerbose() << "NPCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    ofSetColor(ofColor::white);
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
}


BiggerFish::BiggerFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) {
    m_dx = (rand() % 3 - 1);
    m_dy = (rand() % 3 - 1);
    normalize();

    setCollisionRadius(60); // Bigger fish have a larger collision radius
    m_value = 5; // Bigger fish have a higher value
    m_creatureType = AquariumCreatureType::BiggerFish;
}

void BiggerFish::move() {
    // Bigger fish might move slower or have different logic
    m_x += m_dx * (m_speed * 0.5); // Moves at half speed
    m_y += m_dy * (m_speed * 0.5);
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }

    this->handleBounds();
}

void BiggerFish::draw() const {
    ofLogVerbose() << "BiggerFish at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    this->m_sprite->draw(this->m_x, this->m_y);
}


// AquariumSpriteManager
AquariumSpriteManager::AquariumSpriteManager(){
    this->m_npc_fish = std::make_shared<GameSprite>("base-fish.png", 70,70);
    this->m_big_fish = std::make_shared<GameSprite>("bigger-fish.png", 120, 120);
    this->m_pescao_cute = std::make_shared<GameSprite>("pescao-cute.png", 60, 60);
    this->m_clown_fish = std::make_shared<GameSprite>("pez-payaso-pixelao.png", 55, 55);
}

std::shared_ptr<GameSprite> AquariumSpriteManager::GetSprite(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return std::make_shared<GameSprite>(*this->m_big_fish);
        case AquariumCreatureType::NPCreature:
            return std::make_shared<GameSprite>(*this->m_npc_fish);
        case AquariumCreatureType::PescaoCute:
            return std::make_shared<GameSprite>(*this->m_pescao_cute);
        case AquariumCreatureType::ClownFish:
            return std::make_shared<GameSprite>(*this->m_clown_fish);
        default:
            return nullptr;
    }
}


// Aquarium Implementation
Aquarium::Aquarium(int width, int height, std::shared_ptr<AquariumSpriteManager> spriteManager)
    : m_width(width), m_height(height) {
        m_sprite_manager =  spriteManager;
    }



void Aquarium::addCreature(std::shared_ptr<Creature> creature) {
    creature->setBounds(m_width - 20, m_height - 20);
    m_creatures.push_back(creature);
}

void Aquarium::addAquariumLevel(std::shared_ptr<AquariumLevel> level){
    if(level == nullptr){return;} // guard to not add noise
    this->m_aquariumlevels.push_back(level);
}

void Aquarium::update() {

    for (auto& creature : m_creatures) {
        creature->move();
    }

    for (size_t i = 0; i < m_creatures.size(); ++i) {
        for (size_t j = i + 1; j < m_creatures.size(); ++j) {
            auto a = m_creatures[i];
            auto b = m_creatures[j];

            if (!std::dynamic_pointer_cast<NPCreature>(a) || !std::dynamic_pointer_cast<NPCreature>(b)) {
                continue;
            }
// Revisa colisiones
            if (checkCollision(a, b)) {

                a->bounce();
                b->bounce();

                a->move();
                b->move();

                ofLogVerbose() << "NPC-NPC collision at (" 
                               << a->getX() << "," << a->getY() << ") and (" 
                               << b->getX() << "," << b->getY() << ")" << std::endl;
            }
        }
    }
    this->Repopulate();
}


void Aquarium::draw() const {
    for (const auto& creature : m_creatures) {
        creature->draw();
    }
}


void Aquarium::removeCreature(std::shared_ptr<Creature> creature) {
    auto it = std::find(m_creatures.begin(), m_creatures.end(), creature);
    if (it != m_creatures.end()) {
        ofLogVerbose() << "removing creature " << endl;
        int selectLvl = this->currentLevel % this->m_aquariumlevels.size();
        auto npcCreature = std::static_pointer_cast<NPCreature>(creature);
        this->m_aquariumlevels.at(selectLvl)->ConsumePopulation(npcCreature->GetType(), npcCreature->getValue());
        m_creatures.erase(it);
    }
}

void Aquarium::clearCreatures() {
    m_creatures.clear();
}

std::shared_ptr<Creature> Aquarium::getCreatureAt(int index) {
    if (index < 0 || size_t(index) >= m_creatures.size()) {
        return nullptr;
    }
    return m_creatures[index];
}



void Aquarium::SpawnCreature(AquariumCreatureType type) {
    int x = rand() % this->getWidth();
    int y = rand() % this->getHeight();
    int speed = 1 + rand() % 25; // Speed between 1 and 25

    switch (type) {
        case AquariumCreatureType::NPCreature:
            this->addCreature(std::make_shared<NPCreature>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::NPCreature)));
            break;
        case AquariumCreatureType::BiggerFish:
            this->addCreature(std::make_shared<BiggerFish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::BiggerFish)));
            break;
        case AquariumCreatureType::PescaoCute:
            this->addCreature(std::make_shared<PescaoCute>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::PescaoCute)));
            break;
        case AquariumCreatureType::ClownFish:
            this->addCreature(std::make_shared<ClownFish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::ClownFish)));
            break;
        default:
            ofLogError() << "Unknown creature type to spawn!";
            break;
    }
}


// repopulation will be called from the levl class
// it will compose into aquarium so eating eats frm the pool of NPCs in the lvl class
// once lvl criteria met, we move to new lvl through inner signal asking for new lvl
// which will mean incrementing the buffer and pointing to a new lvl index
void Aquarium::Repopulate() {
    ofLogVerbose("entering phase repopulation");
    // lets make the levels circular
    int selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
    ofLogVerbose() << "the current index: " << selectedLevelIdx << endl;
    std::shared_ptr<AquariumLevel> level = this->m_aquariumlevels.at(selectedLevelIdx);


    if(level->isCompleted()){
        level->levelReset();
        this->currentLevel += 1;
        selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
        ofLogNotice()<<"new level reached : " << selectedLevelIdx << std::endl;
        level = this->m_aquariumlevels.at(selectedLevelIdx);
        this->clearCreatures();
    }

    
    // now lets find how many to respawn if needed 
    std::vector<AquariumCreatureType> toRespawn = level->Repopulate();
    ofLogVerbose() << "amount to repopulate : " << toRespawn.size() << endl;
    if(toRespawn.size() <= 0 ){return;} // there is nothing for me to do here
    for(AquariumCreatureType newCreatureType : toRespawn){
        this->SpawnCreature(newCreatureType);
    }
}


// Aquarium collision detection
std::shared_ptr<GameEvent> DetectAquariumCollisions(std::shared_ptr<Aquarium> aquarium, std::shared_ptr<PlayerCreature> player) {
    if (!aquarium || !player) return nullptr;
    
    for (int i = 0; i < aquarium->getCreatureCount(); ++i) {
        std::shared_ptr<Creature> npc = aquarium->getCreatureAt(i);
        if (npc && checkCollision(player, npc)) {
            return std::make_shared<GameEvent>(GameEventType::COLLISION, player, npc);
        }
    }
    return nullptr;
};

AquariumGameScene::AquariumGameScene(std::shared_ptr<PlayerCreature> player,
                                     std::shared_ptr<Aquarium> aquarium,
                                     string name)
: m_player(std::move(player))
, m_aquarium(std::move(aquarium))
, m_name(std::move(name))
{
}


//  Imlementation of the AquariumScene

void AquariumGameScene::Update(){
    std::shared_ptr<GameEvent> event;

    // Check power-up based on score
    int currentScore = this->m_player->getScore();
    if (currentScore > 0 && currentScore % 10 == 0 && !this->m_player->isPoweredUp()) {
        this->m_player->activatePowerUp();
    }

    this->m_player->update();

    if (this->updateControl.tick()) {
        event = DetectAquariumCollisions(this->m_aquarium, this->m_player);
        if (event != nullptr && event->isCollisionEvent()) {
            ofLogVerbose() << "Collision detected between player and NPC!" << std::endl;
            if(event->creatureB != nullptr){
                event->print();
                if(this->m_player->getPower() < event->creatureB->getValue()){
                    ofLogNotice() << "Player is too weak to eat the creature!" << std::endl;
                    this->m_player->loseLife(3*60); // 3 frames debounce, 3 seconds at 60fps
                    if(this->m_player->getLives() <= 0){
                        this->m_lastEvent = std::make_shared<GameEvent>(GameEventType::GAME_OVER, this->m_player, nullptr);
                        return;
                    }
                }
                else{
                    this->m_aquarium->removeCreature(event->creatureB);
                    this->m_player->addToScore(1, event->creatureB->getValue());

                    auto npc = std::dynamic_pointer_cast<NPCreature>(event->creatureB);
                    if (npc) {
                        AquariumCreatureType type = npc->GetType();
                        if (type == AquariumCreatureType::PescaoCute) {
                            if (this->m_player->getLives() < 5) {
                                this->m_player->setLives(this->m_player->getLives() + 1);
                                ofLogNotice() << "Bonus! Extra life from PescaoCute. Lives: "
                                                << this->m_player->getLives();

                            }
                        }
                        if (type == AquariumCreatureType::ClownFish) {
                            this->m_player->addToScore(2); // +2 extra
                            ofLogNotice() << "Bonus! Extra score from ClownFish";
                        }

                    }

                    if (this->m_player->getScore() % 25 == 0){
                        this->m_player->increasePower(1);
                        ofLogNotice() << "Player power increased to " << this->m_player->getPower() << "!" << std::endl;
                    }
                    
                }
                
                

            } else {
                ofLogError() << "Error: creatureB is null in collision event." << std::endl;
            }
        }
        this->m_aquarium->update();
    }

}

void AquariumGameScene::Draw() {
    this->m_player->draw();
    this->m_aquarium->draw();
    this->paintAquariumHUD();

}


void AquariumGameScene::paintAquariumHUD(){
    float panelWidth = ofGetWindowWidth() - 180;
    float t = ofGetElapsedTimef();
    float pulse = (sin(t * 3.0f) + 1.0f) * 0.5f; 

    
    int r = 30 + (int)(pulse * 50);
    int g = 100 + (int)(pulse * 100);
    int b = 200 + (int)(pulse * 55);

    ofSetColor(r, g, b, 180);
    ofDrawRectangle(panelWidth - 30, 5, 170, 70);
    ofNoFill();
    ofSetColor(255, 255, 255, 200);
    ofDrawRectangle(panelWidth - 30, 5, 170, 70);
    ofFill();


    if (this->m_player->isPoweredUp()) {
        ofSetColor(255, 255, 0); 
    } else {
        ofSetColor(255);
    }

    
    ofDrawBitmapString("Score: " + std::to_string(this->m_player->getScore()), panelWidth, 25);
    ofDrawBitmapString("Power: " + std::to_string(this->m_player->getPower()), panelWidth, 40);
    ofDrawBitmapString("Lives: " + std::to_string(this->m_player->getLives()), panelWidth, 55);

    
    for (int i = 0; i < this->m_player->getLives(); ++i) {
        ofSetColor(255, 50, 50);
        ofDrawCircle(panelWidth + i * 20, 65, 5);
    }

    ofSetColor(255); 
}


void AquariumLevel::populationReset(){
    for(auto node: this->m_levelPopulation){
        node->currentPopulation = 0; // need to reset the population to ensure they are made a new in the next level
    }
}

void AquariumLevel::ConsumePopulation(AquariumCreatureType creatureType, int power){
    for(std::shared_ptr<AquariumLevelPopulationNode> node: this->m_levelPopulation){
        ofLogVerbose() << "consuming from this level creatures" << endl;
        if(node->creatureType == creatureType){
            ofLogVerbose() << "-cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            if(node->currentPopulation == 0){
                return;
            } 
            node->currentPopulation -= 1;
            ofLogVerbose() << "+cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            this->m_level_score += power;
            return;
        }
    }
}

bool AquariumLevel::isCompleted(){
    return this->m_level_score >= this->m_targetScore;
}

std::vector<AquariumCreatureType> AquariumLevel::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;

    for (std::shared_ptr<AquariumLevelPopulationNode> node : this->m_levelPopulation) {
        int delta = node->population - node->currentPopulation;
        ofLogVerbose() << "to Repopulate :  " << delta << endl;

        if (delta > 0) {
            for (int i = 0; i < delta; i++) {
                toRepopulate.push_back(node->creatureType);
            }
            node->currentPopulation += delta;
        }
    }

    return toRepopulate;
}







//PescaoCute
PescaoCute::PescaoCute(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) {
    m_dx = (rand() % 3 - 1);
    m_dy = (rand() % 3 - 1);
    normalize();
    m_value = 2;
    m_creatureType = AquariumCreatureType::PescaoCute;
}

void PescaoCute::move() {
    if (directionChangeTimer.tick()) {
        //Que cambie de dirección al moverse
        m_dx = (rand() % 3 - 1) * 1.5f;
        m_dy = (rand() % 3 - 1) * 1.5f;
        normalize();
    }
    
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    
    if(m_dx < 0) {
        m_sprite->setFlipped(true);
    } else {
        m_sprite->setFlipped(false);
    }
    
    handleBounds();
}

void PescaoCute::draw() const {
    m_sprite->draw(m_x, m_y);
}

//ClownFish
ClownFish::ClownFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) {
    m_dx = (rand() % 3 - 1);
    m_dy = (rand() % 3 - 1);
    normalize();
    m_value = 2;
    m_creatureType = AquariumCreatureType::ClownFish;
}

void ClownFish::move() {
    //Movimiento base pero más lento
    m_x += m_dx * (m_speed * 0.75f);
    m_y += m_dy * (m_speed * 0.75f);
    
    if(m_dx < 0) {
        m_sprite->setFlipped(true);
    } else {
        m_sprite->setFlipped(false);
    }
    
    handleBounds();
}

void ClownFish::draw() const {
    m_sprite->draw(m_x, m_y);
}



