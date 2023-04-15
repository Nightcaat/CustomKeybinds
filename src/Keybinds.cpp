#include "../include/Keybinds.hpp"
#include <Geode/utils/ranges.hpp>
#include <Geode/utils/string.hpp>

using namespace geode::prelude;
using namespace keybinds;

Device::~Device() {
    BindManager::get()->detachDevice(this);
}

class KeyboardDevice final : public Device {
public:
    std::string getID() const override {
        return "keyboard"_spr;
    }

    Bind* loadBind(std::string const& data) override {
        int mods, key;
        std::stringstream ss(data);
        ss >> mods;
        if (ss.get() != '|') {
            return nullptr;
        }
        ss >> key;
        if (ss.fail()) {
            return nullptr;
        }
        return Keybind::create(static_cast<enumKeyCodes>(key), static_cast<Modifier>(mods));
    }

    std::string saveBind(Bind* bind) override {
        auto key = static_cast<Keybind*>(bind);
        return std::to_string(static_cast<int>(key->getModifiers())) + "|" +
            std::to_string(key->getKey());
    }

    static KeyboardDevice* get() {
        static auto inst = new KeyboardDevice();
        return inst;
    }
};

class ControllerDevice final : public Device {
public:
    std::string getID() const override {
        return "controller"_spr;
    }

    Bind* loadBind(std::string const& data) override {
        int key;
        std::stringstream ss(data);
        ss >> key;
        if (ss.fail()) {
            return nullptr;
        }
        return ControllerBind::create(static_cast<enumKeyCodes>(key));
    }

    std::string saveBind(Bind* bind) override {
        return std::to_string(static_cast<ControllerBind*>(bind)->getButton());
    }

    static ControllerDevice* get() {
        static auto inst = new ControllerDevice();
        return inst;
    }
};

Modifier keybinds::operator|=(Modifier& a, Modifier const& b) {
    return static_cast<Modifier>(reinterpret_cast<int&>(a) |= static_cast<int>(b));
}

Modifier keybinds::operator|(Modifier const& a, Modifier const& b) {
    return static_cast<Modifier>(static_cast<int>(a) | static_cast<int>(b));
}

bool keybinds::operator&(Modifier const& a, Modifier const& b) {
    return static_cast<bool>(static_cast<int>(a) & static_cast<int>(b));
}

std::string keybinds::keyToString(enumKeyCodes key) {
    switch (key) {
        case KEY_None:      return "";
        case KEY_C:         return "C";
        case KEY_Multiply:  return "Mul";
        case KEY_Divide:    return "Div";
        case KEY_OEMPlus:   return "Plus";
        case KEY_OEMMinus:  return "Minus";
        case static_cast<enumKeyCodes>(-1): return "Unk";
        default: return CCKeyboardDispatcher::get()->keyToString(key);
    }
}

bool keybinds::keyIsModifier(enumKeyCodes key) {
    return
        key == KEY_Control ||
        key == KEY_LeftControl ||
        key == KEY_RightContol ||
        key == KEY_Shift ||
        key == KEY_LeftShift ||
        key == KEY_RightShift ||
        key == KEY_Alt || 
        key == KEY_LeftWindowsKey ||
        key == KEY_RightWindowsKey;
}

bool keybinds::keyIsController(enumKeyCodes key) {
    return key >= CONTROLLER_A && key <= CONTROLLER_Right;
}

bool Bind::isEqual(Bind* other) const {
    return this->getHash() == other->getHash();
}

CCNode* Bind::createLabel() const {
    return CCLabelBMFont::create(this->toString().c_str(), "goldFont.fnt");
}

CCNode* Bind::createBindSprite() const {
    auto bg = CCScale9Sprite::create("square.png"_spr);
    bg->setOpacity(85);
    bg->setScale(.45f);

    auto top = this->createLabel();
    limitNodeSize(top, { 125.f, 30.f }, 1.f, .1f);
    bg->setContentSize({
        clamp(top->getScaledContentSize().width + 18.f, 18.f / bg->getScale(), 145.f),
        18.f / bg->getScale()
    });
    bg->addChild(top);

    top->setPosition(bg->getContentSize() / 2);
    
    return bg;
}

Keybind* Keybind::create(enumKeyCodes key, Modifier modifiers) {
    if (key == KEY_None || key == KEY_Unknown || keyIsController(key)) {
        return nullptr;
    }
    auto ret = new Keybind(); 
    ret->m_key = key;
    ret->m_modifiers = modifiers;
    ret->autorelease();
    return ret;
}

enumKeyCodes Keybind::getKey() const {
    return m_key;
}

Modifier Keybind::getModifiers() const {
    return m_modifiers;
}

size_t Keybind::getHash() const {
    return m_key | (static_cast<int>(m_modifiers) << 29);
}

bool Keybind::isEqual(Bind* other) const {
    if (auto o = typeinfo_cast<Keybind*>(other)) {
        return m_key == o->m_key && m_modifiers == o->m_modifiers;
    }
    return false;
}

std::string Keybind::toString() const {
    std::string res = "";
    if (m_modifiers & Modifier::Control) {
        res += "Ctrl + ";
    }
    if (m_modifiers & Modifier::Command) {
        res += "Cmd + ";
    }
    if (m_modifiers & Modifier::Shift) {
        res += "Shift + ";
    }
    if (m_modifiers & Modifier::Alt) {
        res += "Alt + ";
    }
    res += keyToString(m_key);
    return res;
}

std::string Keybind::getDeviceID() const {
    return "keyboard"_spr;
}

ControllerBind* ControllerBind::create(enumKeyCodes button) {
    if (!keyIsController(button)) {
        return nullptr;
    }
    auto ret = new ControllerBind(); 
    ret->m_button = button;
    ret->autorelease();
    return ret;
}

enumKeyCodes ControllerBind::getButton() const {
    return m_button;
}

size_t ControllerBind::getHash() const {
    return m_button;
}

bool ControllerBind::isEqual(Bind* other) const {
    if (auto o = typeinfo_cast<ControllerBind*>(other)) {
        return m_button == o->m_button;
    }
    return false;
}

std::string ControllerBind::toString() const {
    return keyToString(m_button);
}

CCNode* ControllerBind::createLabel() const {
    const char* sprite = nullptr;
    switch (m_button) {
        case CONTROLLER_A: sprite = "controllerBtn_A_001.png"; break;
        case CONTROLLER_B: sprite = "controllerBtn_B_001.png"; break;
        case CONTROLLER_X: sprite = "controllerBtn_X_001.png"; break;
        case CONTROLLER_Y: sprite = "controllerBtn_Y_001.png"; break;
        case CONTROLLER_Back: sprite = "controllerBtn_Back_001.png"; break;
        case CONTROLLER_Start: sprite = "controllerBtn_Start_001.png"; break;
        case CONTROLLER_Down: sprite = "controllerBtn_DPad_Down_001.png"; break;
        case CONTROLLER_Left: sprite = "controllerBtn_DPad_Left_001.png"; break;
        case CONTROLLER_Up: sprite = "controllerBtn_DPad_Up_001.png"; break;
        case CONTROLLER_Right: sprite = "controllerBtn_DPad_Right_001.png"; break;
        case CONTROLLER_LT: sprite = "controllerBtn_LThumb_001.png"; break;
        case CONTROLLER_RT: sprite = "controllerBtn_RThumb_001.png"; break;
        // todo: are these the same
        case CONTROLLER_LB: sprite = "controllerBtn_LThumb_001.png"; break;
        case CONTROLLER_RB: sprite = "controllerBtn_RThumb_001.png"; break;
    }
    if (!sprite) {
        return CCLabelBMFont::create("Unk", "goldFont.fnt");
    }
    return CCSprite::createWithSpriteFrameName(sprite);
}

std::string ControllerBind::getDeviceID() const {
    return "controller"_spr;
}

BindHash::BindHash(Bind* bind) : bind(bind) {}

bool BindHash::operator==(BindHash const& other) const {
    return bind->isEqual(other.bind);
}

std::size_t std::hash<keybinds::BindHash>::operator()(keybinds::BindHash const& hash) const {
    return hash.bind ? (hash.bind->getHash() | std::hash<std::string>()(hash.bind->getDeviceID())) : 0;
}

Category::Category(const char* path) : m_value(path) {}

Category::Category(std::string const& path) : m_value(path) {}

std::vector<std::string> Category::getPath() const {
    return string::split(m_value, "/");
}

std::optional<Category> Category::getParent() const {
    if (string::contains(m_value, '/')) {
        return Category(m_value.substr(0, m_value.find_last_of('/')));
    }
    return std::nullopt;
}

bool Category::hasParent(Category const& parent) const {
    return m_value.starts_with(parent.m_value);
}

std::string Category::toString() const {
    return m_value;
}

bool Category::operator==(Category const& other) const {
    return m_value == other.m_value;
}

std::string BindableAction::getID() const {
    return m_id;
}

std::string BindableAction::getName() const {
    return m_name.empty() ? m_id : m_name;
}

std::string BindableAction::getDescription() const {
    return m_description;
}

Mod* BindableAction::getMod() const {
    return m_owner;
}

std::vector<Ref<Bind>> BindableAction::getDefaults() const {
    return m_defaults;
}

Category BindableAction::getCategory() const {
    return m_category;
}

bool BindableAction::isRepeatable() const {
    return m_repeatable;
}

BindableAction::BindableAction(
    ActionID const& id,
    std::string const& name,
    std::string const& description,
    std::vector<Ref<Bind>> const& defaults,
    Category const& category,
    bool repeatable,
    Mod* owner
) : m_id(id),
    m_owner(owner),
    m_name(name),
    m_description(description),
    m_category(category),
    m_repeatable(repeatable),
    m_defaults(defaults) {}

InvokeBindEvent::InvokeBindEvent(ActionID const& id, bool down) : m_id(id), m_down(down) {}

std::string InvokeBindEvent::getID() const {
    return m_id;
}

bool InvokeBindEvent::isDown() const {
    return m_down;
}

ListenerResult InvokeBindFilter::handle(utils::MiniFunction<Callback> fn, InvokeBindEvent* event) {
    if (event->getID() == m_id) {
        return fn(event);
    }
    return ListenerResult::Propagate;
}

InvokeBindFilter::InvokeBindFilter(CCNode* target, ActionID const& id)
  : m_target(target), m_id(id) {
    BindManager::get()->stopAllRepeats();
}

PressBindEvent::PressBindEvent(Bind* bind, bool down) : m_bind(bind), m_down(down) {}

Bind* PressBindEvent::getBind() const {
    return m_bind;
}

bool PressBindEvent::isDown() const {
    return m_down;
}

geode::ListenerResult PressBindFilter::handle(geode::utils::MiniFunction<Callback> fn, PressBindEvent* event) {
    return fn(event);
}

PressBindFilter::PressBindFilter() {}

BindManager::BindManager() {
    this->addCategory(Category::GLOBAL);
    this->addCategory(Category::PLAY);
    this->addCategory(Category::EDITOR);
    this->attachDevice(KeyboardDevice::get());
    this->retain();
}

BindManager* BindManager::get() {
    static auto inst = new BindManager();
    return inst;
}

void BindManager::attachDevice(Device* device) {
    this->detachDevice(device);
    m_devices.insert({ device->getID(), device });
    for (auto& [id, actions] : m_devicelessBinds) {
        if (id != device->getID()) {
            continue;
        }
        for (auto& [action, binds] : actions) {
            for (auto& data : binds) {
                if (auto nbind = device->loadBind(data)) {
                    this->addBindTo(action, nbind);
                }
            }
            binds.clear();
        }
    }
}

void BindManager::detachDevice(Device* device) {
    for (auto& [bind, actions] : m_binds) {
        if (bind.bind->getDeviceID() == device->getID()) {
            for (auto& action : actions) {
                m_devicelessBinds[device->getID()][action].push_back(device->saveBind(bind.bind));
                this->removeBindFrom(action, bind.bind);
            }
        }
    }
    m_devices.erase(device->getID());
}

std::string BindManager::getBindSaveString(Bind* bind) const {
    auto dev = bind->getDeviceID();
    if (m_devices.contains(dev)) {
        return dev + ":" + m_devices.at(dev)->saveBind(bind);
    }
    return "";
}

std::pair<DeviceID, std::string> BindManager::parseBindSave(std::string const& str) const {
    auto off = str.find_first_of(':');
    if (off == std::string::npos) {
        return { "", "" };
    }
    return { str.substr(0, off), str.substr(off + 1) };
}

Bind* BindManager::loadBindFromSaveString(std::string const& data) const {
    auto [id, sdata] = this->parseBindSave(data);
    if (!m_devices.contains(id)) {
        return nullptr;
    }
    return m_devices.at(id)->loadBind(sdata);
}

bool BindManager::loadActionBinds(ActionID const& action) {
    try {
        auto value = Mod::get()->template getSavedValue<json::Object>(action);
        for (auto bind : value["binds"].as_array()) {
            // try directly parsing the bind from a string if the device it's for 
            // is already connected
            if (auto b = BindManager::get()->loadBindFromSaveString(bind.as_string())) {
                this->addBindTo(action, b);
            }
            // otherwise save the bind's data for until the device is connected 
            // or the game is closed
            else {
                // get the device ID and bind save data from the raw string
                auto [id, data] = this->parseBindSave(bind.as_string());
                // if device ID has a size, then add this to the list of unbound 
                // binds
                if (id.size()) {
                    m_devicelessBinds[id][action].push_back(data);
                }
                // otherwise invalid bind save data
            }
        }
        // load repeat options
        if (value.count("repeat")) {
            auto rep = value["repeat"].as_object();
            auto opts = RepeatOptions();
            opts.enabled = rep["enabled"].as_bool();
            opts.rate = rep["rate"].as_int();
            opts.delay = rep["delay"].as_int();
            this->setRepeatOptionsFor(action, opts);
        }
        return true;
    }
    catch(...) {
        return false;
    }
}

void BindManager::saveActionBinds(ActionID const& action) {
    auto obj = json::Object();
    auto binds = json::Array();
    for (auto& bind : this->getBindsFor(action)) {
        binds.push_back(this->getBindSaveString(bind));
    }
    for (auto& [device, actions] : m_devicelessBinds) {
        if (actions.contains(action)) {
            for (auto& bind : actions.at(action)) {
                binds.push_back(device + ":" + bind);
            }
        }
    }
    obj["binds"] = binds;
    if (auto opts = this->getRepeatOptionsFor(action)) {
        auto rep = json::Object();
        rep["enabled"] = opts.value().enabled;
        rep["rate"] = opts.value().rate;
        rep["delay"] = opts.value().delay;
        obj["repeat"] = rep;
    }
}

bool BindManager::registerBindable(BindableAction const& action, ActionID const& after) {
    this->stopAllRepeats();
    if (ranges::contains(m_actions, [&](auto const& act) { return act.first == action.getID(); })) {
        return false;
    }
    if (auto ix = ranges::indexOf(m_actions, [&](auto const& a) { return a.first == after; })) {
        m_actions.insert(m_actions.begin() + ix.value() + 1, { 
            action.getID(),
            { .definition = action, .repeat = RepeatOptions() }
        });
    }
    else {
        m_actions.push_back({
            action.getID(),
            { .definition = action, .repeat = RepeatOptions() }
        });
    }
    this->addCategory(action.getCategory());
    if (!this->loadActionBinds(action.getID())) {
        for (auto& def : action.getDefaults()) {
            this->addBindTo(action.getID(), def);
        }
    }
    return true;
}

void BindManager::removeBindable(ActionID const& action) {
    this->stopAllRepeats();
    this->removeAllBindsFrom(action);
    ranges::remove(m_actions, [&](auto const& act) { return act.first == action; });
}

std::optional<BindableAction> BindManager::getBindable(ActionID const& action) const {
    for (auto& [id, bindable] : m_actions) {
        if (id == action) {
            return bindable.definition;
        }
    }
    return std::nullopt;
}

std::vector<BindableAction> BindManager::getAllBindables() const {
    std::vector<BindableAction> res;
    for (auto& [_, action] : m_actions) {
        res.push_back(action.definition);
    }
    return res;
}

std::vector<BindableAction> BindManager::getBindablesIn(Category const& category, bool sub) const {
    std::vector<BindableAction> res;
    for (auto& [_, action] : m_actions) {
        if (sub ?
            action.definition.getCategory().hasParent(category) :
            (action.definition.getCategory() == category)
        ) {
            res.push_back(action.definition);
        }
    }
    return res;
}

std::vector<BindableAction> BindManager::getBindablesFor(Bind* bind) const {
    std::vector<BindableAction> res {};
    if (m_binds.count(bind)) {
        for (auto& bindable : m_binds.at(bind)) {
            if (auto action = this->getBindable(bindable)) {
                res.push_back(action.value());
            }
        }
    }
    return res;
}

std::vector<Category> BindManager::getAllCategories() const {
    return m_categories;
}

void BindManager::addCategory(Category const& category) {
    this->stopAllRepeats();
    if (!ranges::contains(m_categories, category)) {
        // Add parent categories first if they don't exist yet
        if (auto parent = category.getParent()) {
            this->addCategory(parent.value());
        }
        auto it = m_categories.begin();
        bool foundParent = false;
        for (auto& cat : m_categories) {
            if (cat.hasParent(category)) {
                foundParent = true;
            }
            else if (foundParent) {
                break;
            }
            it++;
        }
        m_categories.insert(it, category);
    }
}

void BindManager::removeCategory(Category const& category) {
    this->stopAllRepeats();
    for (auto& bindable : this->getBindablesIn(category, true)) {
        this->removeBindable(bindable.getID());
    }
    ranges::remove(m_categories, [&](auto const& cat) { return cat.hasParent(category); });
}

void BindManager::addBindTo(ActionID const& action, Bind* bind) {
    this->stopAllRepeats();
    m_binds[bind].push_back(action);
}

void BindManager::removeBindFrom(ActionID const& action, Bind* bind) {
    this->stopAllRepeats();
    ranges::remove(m_binds[bind], action);
    if (m_binds.at(bind).empty()) {
        m_binds.erase(bind);
    }
}

void BindManager::removeAllBindsFrom(ActionID const& action) {
    this->stopAllRepeats();
    for (auto& [bind, actions] : m_binds) {
        ranges::remove(actions, action);
    }
}

std::vector<Ref<Bind>> BindManager::getBindsFor(ActionID const& action) const {
    std::vector<Ref<Bind>> binds;
    for (auto& [bind, actions] : m_binds) {
        if (ranges::contains(actions, action)) {
            binds.push_back(bind.bind);
        }
    }
    return binds;
}

void BindManager::resetBindsToDefault(ActionID const& action) {
    this->stopAllRepeats();
    this->removeAllBindsFrom(action);
    if (auto bindable = this->getBindable(action)) {
        for (auto& def : bindable->getDefaults()) {
            this->addBindTo(action, def);
        }
    }
}

bool BindManager::hasDefaultBinds(ActionID const& action) const {
    if (auto bindable = this->getBindable(action)) {
        auto binds = this->getBindsFor(action);
        auto defs = bindable->getDefaults();
        if (binds.size() == defs.size()) {
            for (size_t i = 0; i < binds.size(); i++) {
                if (!binds.at(i)->isEqual(defs.at(i))) {
                    return false;
                }
            }
            return true;
        }
    }
    return false;
}

std::optional<RepeatOptions> BindManager::getRepeatOptionsFor(ActionID const& action) {
    for (auto& [id, bindable] : m_actions) {
        if (id == action) {
            if (bindable.definition.isRepeatable()) {
                return bindable.repeat;
            }
        }
    }
    return std::nullopt;
}

void BindManager::setRepeatOptionsFor(ActionID const& action, RepeatOptions const& options) {
    this->stopAllRepeats();
    for (auto& [id, bindable] : m_actions) {
        if (id == action) {
            bindable.repeat = options;
        }
    }
}

ListenerResult BindManager::onDispatch(PressBindEvent* event) {
    if (m_binds.contains(event->getBind())) {
        for (auto& action : m_binds.at(event->getBind())) {
            if (event->isDown()) {
                this->repeat(action);
            }
            else {
                this->unrepeat(action);
            }
            if (InvokeBindEvent(action, event->isDown()).post() == ListenerResult::Stop) {
                return ListenerResult::Stop;
            }
        }
    }
    return ListenerResult::Propagate;
}

void BindManager::stopAllRepeats() {
    m_repeating.clear();
    CCScheduler::get()->unscheduleSelector(
        schedule_selector(BindManager::onRepeat), this
    );
}

void BindManager::unrepeat(ActionID const& action) {
    ranges::remove(m_repeating, [=](auto const& p) { return p.first == action; });
}

void BindManager::repeat(ActionID const& action) {
    if (auto options = this->getRepeatOptionsFor(action)) {
        if (options.value().enabled) {
            m_repeating.emplace_back(action, options.value().delay / 1000.f);
            CCScheduler::get()->scheduleSelector(
                schedule_selector(BindManager::onRepeat), this,
                0.f, false
            );
        }
    }
}

void BindManager::onRepeat(float dt) {
    for (auto& [id, last] : m_repeating) {
        if (auto options = this->getRepeatOptionsFor(id)) {
            last -= dt;
            if (last < 0.f) {
                InvokeBindEvent(id, true).post();
                last += options.value().rate / 1000.f;
            }
        }
    }
}

void BindManager::save() {
    for (auto& bindable : BindManager::get()->getAllBindables()) {
        BindManager::get()->saveActionBinds(bindable.getID());
    }
}

$on_mod(DataSaved) {
    BindManager::get()->save();
}
