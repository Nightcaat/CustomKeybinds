#pragma once

#include <Geode/DefaultInclude.hpp>
#include <Geode/utils/MiniFunction.hpp>
#include <Geode/utils/cocos.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/loader/Event.hpp>
#include <cocos2d.h>

#ifdef GEODE_IS_WINDOWS
    #ifdef HJFOD_CUSTOM_KEYBINDS_EXPORTING
        #define CUSTOM_KEYBINDS_DLL __declspec(dllexport)
    #else
        #define CUSTOM_KEYBINDS_DLL __declspec(dllimport)
    #endif
#else
    #define CUSTOM_KEYBINDS_DLL
#endif

struct BindSaveData;

namespace keybinds {
    class Bind;
    class BindManager;

    using DeviceID = std::string;

    class CUSTOM_KEYBINDS_DLL Device {
    public:
        virtual DeviceID getID() const = 0;
        virtual Bind* loadBind(std::string const& data) = 0;
        virtual std::string saveBind(Bind* bind) = 0;
        virtual ~Device();
    };

    /**
     * Base class for implementing bindings for different input devices
     */
    class CUSTOM_KEYBINDS_DLL Bind : public cocos2d::CCObject {
    protected:
        friend class BindManager;

    public:
        /**
         * Get the hash for this bind
         */
        virtual size_t getHash() const = 0;
        /**
         * Check if this bind is equal to another. By default compares hashes
         */
        virtual bool isEqual(Bind* other) const;
        /**
         * Get the bind's representation as a string
         */
        virtual std::string toString() const = 0;
        virtual cocos2d::CCNode* createLabel() const;
        virtual DeviceID getDeviceID() const = 0;
        virtual ~Bind() = default;

        cocos2d::CCNode* createBindSprite() const;
    };

    enum class Modifier : unsigned int {
        None        = 0b0000,
        Control     = 0b0001,
        Shift       = 0b0010,
        Alt         = 0b0100,
        Command     = 0b1000,
    };
    CUSTOM_KEYBINDS_DLL Modifier operator|(Modifier const& a, Modifier const& b);
    CUSTOM_KEYBINDS_DLL Modifier operator|=(Modifier& a, Modifier const& b);
    CUSTOM_KEYBINDS_DLL bool operator&(Modifier const& a, Modifier const& b);

    CUSTOM_KEYBINDS_DLL std::string keyToString(cocos2d::enumKeyCodes key);
    CUSTOM_KEYBINDS_DLL bool keyIsModifier(cocos2d::enumKeyCodes key);
    CUSTOM_KEYBINDS_DLL bool keyIsController(cocos2d::enumKeyCodes key);

    class CUSTOM_KEYBINDS_DLL Keybind final : public Bind {
    protected:
        cocos2d::enumKeyCodes m_key;
        Modifier m_modifiers;

    public:
        static Keybind* create(cocos2d::enumKeyCodes key, Modifier modifiers = Modifier::None);

        cocos2d::enumKeyCodes getKey() const;
        Modifier getModifiers() const;

        size_t getHash() const override;
        bool isEqual(Bind* other) const override;
        std::string toString() const override;
        DeviceID getDeviceID() const override;
    };

    class CUSTOM_KEYBINDS_DLL ControllerBind final : public Bind {
    protected:
        cocos2d::enumKeyCodes m_button;
    
    public:
        static ControllerBind* create(cocos2d::enumKeyCodes button);

        cocos2d::enumKeyCodes getButton() const;

        size_t getHash() const override;
        bool isEqual(Bind* other) const override;
        std::string toString() const override;
        cocos2d::CCNode* createLabel() const override;
        DeviceID getDeviceID() const override;
    };

    struct CUSTOM_KEYBINDS_DLL BindHash {
        geode::Ref<Bind> bind;
        BindHash(Bind* bind);
        bool operator==(BindHash const& other) const;
    };
}

namespace std {
    template<>
    struct hash<keybinds::BindHash> {
        CUSTOM_KEYBINDS_DLL std::size_t operator()(keybinds::BindHash const&) const;
    };
}

namespace keybinds {
    class BindManager;
    class InvokeBindFilter;

    using ActionID = std::string;
    class CUSTOM_KEYBINDS_DLL Category final {
        std::string m_value;
    
    public:
        Category() = default;
        Category(const char* path);
        Category(std::string const& path);
        std::vector<std::string> getPath() const;
        std::optional<Category> getParent() const;
        bool hasParent(Category const& parent) const;
        std::string toString() const;

        bool operator==(Category const&) const;

        static constexpr auto PLAY { "Play" };
        static constexpr auto EDITOR { "Editor" };
        static constexpr auto GLOBAL { "Global" };
        static constexpr auto EDITOR_UI { "Editor/UI" };
        static constexpr auto EDITOR_MODIFY { "Editor/Modify" };
        static constexpr auto EDITOR_MOVE { "Editor/Move" };
    };

    class CUSTOM_KEYBINDS_DLL BindableAction {
    protected:
        ActionID m_id;
        std::string m_name;
        std::string m_description;
        geode::Mod* m_owner;
        std::vector<geode::Ref<Bind>> m_defaults;
        Category m_category;
        bool m_repeatable;
    
    public:
        ActionID getID() const;
        std::string getName() const;
        std::string getDescription() const;
        geode::Mod* getMod() const;
        std::vector<geode::Ref<Bind>> getDefaults() const;
        Category getCategory() const;
        bool isRepeatable() const;

        BindableAction() = default;
        BindableAction(
            ActionID const& id,
            std::string const& name,
            std::string const& description = "",
            std::vector<geode::Ref<Bind>> const& defaults = {},
            Category const& category = Category(),
            bool repeatable = true,
            geode::Mod* owner = geode::Mod::get()
        );
    };

    class CUSTOM_KEYBINDS_DLL InvokeBindEvent : public geode::Event {
    protected:
        ActionID m_id;
        bool m_down;

        friend class BindManager;
        friend class InvokeBindFilter;

    public:
        InvokeBindEvent(ActionID const& id, bool down);
        ActionID getID() const;
        bool isDown() const;
    };

    class CUSTOM_KEYBINDS_DLL InvokeBindFilter : public geode::EventFilter<InvokeBindEvent> {
    protected:
        cocos2d::CCNode* m_target;
        ActionID m_id;

    public:
        using Callback = geode::ListenerResult(InvokeBindEvent*);
        
        geode::ListenerResult handle(geode::utils::MiniFunction<Callback> fn, InvokeBindEvent* event);
        InvokeBindFilter(cocos2d::CCNode* target, ActionID const& id);
    };

    class CUSTOM_KEYBINDS_DLL PressBindEvent : public geode::Event {
    protected:
        Bind* m_bind;
        bool m_down;
    
    public:
        PressBindEvent(Bind* bind, bool down);
        Bind* getBind() const;
        bool isDown() const;
    };

    class CUSTOM_KEYBINDS_DLL PressBindFilter : public geode::EventFilter<PressBindEvent> {
    public:
        using Callback = geode::ListenerResult(PressBindEvent*);
        
        geode::ListenerResult handle(geode::utils::MiniFunction<Callback> fn, PressBindEvent* event);
        PressBindFilter();
    };

    struct CUSTOM_KEYBINDS_DLL RepeatOptions {
        bool enabled = true;
        size_t rate = 300;
        size_t delay = 500;
    };
    
    class CUSTOM_KEYBINDS_DLL BindManager : public cocos2d::CCObject {
    // has to inherit from CCObject for scheduler
    public:
        using DevicelessActions = std::unordered_map<ActionID, std::vector<std::string>>;

    protected:
        struct ActionData {
            BindableAction definition;
            RepeatOptions repeat;
        };
        
        std::unordered_map<BindHash, std::vector<ActionID>> m_binds;
        std::unordered_map<DeviceID, DevicelessActions> m_devicelessBinds;
        std::unordered_map<DeviceID, Device*> m_devices;
        std::vector<std::pair<ActionID, ActionData>> m_actions;
        std::vector<Category> m_categories;
        geode::EventListener<PressBindFilter> m_listener =
            geode::EventListener<PressBindFilter>(this, &BindManager::onDispatch);
        std::vector<std::pair<ActionID, float>> m_repeating;

        BindManager();

        geode::ListenerResult onDispatch(PressBindEvent* event);
        void onRepeat(float dt);
        void repeat(ActionID const& action);
        void unrepeat(ActionID const& action);

        bool loadActionBinds(ActionID const& action);
        void saveActionBinds(ActionID const& action);
        std::pair<DeviceID, std::string> parseBindSave(std::string const& str) const;

        friend class InvokeBindFilter;
        friend struct json::Serialize<BindSaveData>;

    public:
        static BindManager* get();
        void save();

        void attachDevice(Device* device);
        void detachDevice(Device* device);

        std::string getBindSaveString(Bind* bind) const;
        Bind* loadBindFromSaveString(std::string const& str) const;

        bool registerBindable(BindableAction const& action, ActionID const& after = "");
        void removeBindable(ActionID const& action);
        std::optional<BindableAction> getBindable(ActionID const& action) const;
        std::vector<BindableAction> getAllBindables() const;
        std::vector<BindableAction> getBindablesIn(Category const& category, bool sub = false) const;
        std::vector<BindableAction> getBindablesFor(Bind* bind) const;
        std::vector<Category> getAllCategories() const;
        /**
         * Add a new bindable category. If the category is a subcategory (its 
         * ID has a slash, like "Editor/Modify"), then all its parent 
         * categories are inserted aswell, and the subcategory is added after 
         * its parent's last subcategory
         * @param category The category to add. Specify a subcategory by 
         * including a slash in the name (like "Editor/Modify")
         */
        void addCategory(Category const& category);
        /**
         * @note Also removes all the bindables in this category
         */
        void removeCategory(Category const& category);

        void addBindTo(ActionID const& action, Bind* bind);
        void removeBindFrom(ActionID const& action, Bind* bind);
        void removeAllBindsFrom(ActionID const& action);
        void resetBindsToDefault(ActionID const& action);
        bool hasDefaultBinds(ActionID const& action) const;
        std::vector<geode::Ref<Bind>> getBindsFor(ActionID const& action) const;

        std::optional<RepeatOptions> getRepeatOptionsFor(ActionID const& action);
        void setRepeatOptionsFor(ActionID const& action, RepeatOptions const& options);
        void stopAllRepeats();
    };
}
