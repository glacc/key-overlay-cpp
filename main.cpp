#define USE_LINUX_SHOWMETHEKEY_CLI  0
#define USE_SFML_KEY_EVENT          0
#define USE_SFML_KEYPRESSED         1

#include <iostream>
#include <vector>
#include <fstream>
#include <chrono>

#if defined(__linux__)

#if USE_LINUX_SHOWMETHEKEY_CLI
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <gio/gio.h>
#include <pthread.h>
#endif
#endif

#include <yaml-cpp/yaml.h>
#include "magic_enum/magic_enum.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#if 1 || REGION_CONFIGURATION

static const sf::String window_title = "Key Overlay";

static const std::string default_config_file_name = "config.yaml";

// config key names
static const std::string config_font_name_key = "Font";

static const std::string config_framerate_limit_key = "FramerateLimit";

static const std::string config_dpi_scale_enabled_key = "DPIScaleEnabled";
static const std::string config_dpi_scale_reference_key = "DPIScaleReference";

static const std::string config_scale_key = "Scale";

static const std::string config_window_height_key = "WindowHeight";

static const std::string config_margin_left_key = "MarginLeft";
static const std::string config_margin_right_key = "MarginRight";
static const std::string config_margin_bottom_key = "MarginBottom";

static const std::string config_key_size_key = "KeySize";
static const std::string config_key_border_thickness_key = "KeyBorderThickness";
static const std::string config_key_char_size_key = "KeyCharacterSize";
static const std::string config_key_spacing_key = "KeySpacing";

static const std::string config_show_counter_key = "ShowCounter";

static const std::string config_show_key_bpm_key = "ShowKeyBPM1/2";

static const std::string config_key_info_char_size_key = "KeyInfoCharacterSize";
static const std::string config_key_info_position_offset_key = "KeyInfoPositionOffset";
static const std::string config_key_info_color_key = "KeyInfoColor";

static const std::string config_bar_velocity_key = "BarVelocityPerSecond";

static const std::string config_bg_color_key = "BackgroundColor";

static const std::string config_keys_key = "Keys";
static const std::string config_key_key_prefix = "Key";

static const std::string config_key_width_multiplier_key = "WidthMultiplier";
static const std::string config_key_scancode_key = "Scancode";
static const std::string config_key_key_name_key = "KeyName";
static const std::string config_key_color_key = "KeyColor";

struct KeyConfig
{
    float width_multiplier;
    static constexpr float width_multiplier_default = 1.0F;
    
    sf::Keyboard::Scancode scancode;
    static const sf::Keyboard::Scancode scancode_default = sf::Keyboard::Scancode::A;
    sf::String key_name;

    sf::Color color;

    KeyConfig(const float width_multiplier, const sf::Keyboard::Scancode scancode, const sf::String &key_name, const sf::Color color)
        : width_multiplier(width_multiplier), scancode(scancode), key_name(key_name), color(color) { }
};
static const sf::String config_key_key_name_default = L"A";

static const std::vector<KeyConfig> default_keys
({
    KeyConfig(1.0F, sf::Keyboard::Scancode::S, "S", sf::Color::White),
    KeyConfig(1.0F, sf::Keyboard::Scancode::D, "D", sf::Color::Cyan),
    KeyConfig(1.0F, sf::Keyboard::Scancode::F, "F", sf::Color::White),
    KeyConfig(2.0F, sf::Keyboard::Scancode::Space, "Space", sf::Color::Yellow),
    KeyConfig(1.0F, sf::Keyboard::Scancode::J, "J", sf::Color::White),
    KeyConfig(1.0F, sf::Keyboard::Scancode::K, "K", sf::Color::Cyan),
    KeyConfig(1.0F, sf::Keyboard::Scancode::L, "L", sf::Color::White),
});

struct Config
{
    int framerate_limit;
    static constexpr int framerate_limit_default = -1;

    float dpi_scale;

    bool dpi_scale_enabled;
    float dpi_scale_reference;

    float scale;

    std::string font_name;

    int window_height;
    static constexpr int window_height_default = 640;

    int margin_left;
    static constexpr int margin_left_default = 32;
    int margin_right;
    static constexpr int margin_right_default = 32;
    int margin_bottom;
    static constexpr int margin_bottom_default = 64;

    sf::Color bg_color;

    int key_size;
    static constexpr int key_size_default = 64;
    int key_border_thickness;
    static constexpr int key_border_thickness_default = 4;
    unsigned int key_char_size;
    static constexpr unsigned int key_char_size_default = 32U;
    int key_spacing;
    static constexpr int key_spacing_default = 16;

    bool show_counter;
    static constexpr bool show_counter_default = true;

    bool show_key_bpm;
    static constexpr bool show_key_bpm_default = true;

    unsigned int key_info_char_size;
    static constexpr unsigned int key_info_char_size_default = 18U;
    int key_info_position_offset;
    static constexpr int key_info_position_offset_default = 32;
    sf::Color key_info_color;

    int bar_velocity;
    static constexpr int bar_velocity_default = (window_height_default - margin_bottom_default - key_size_default) * 1.5;

    std::vector<KeyConfig> key_configs;

    // dpi scaled values

    float dpi_scale_relative;

    float scale_final;

    float window_height_scaled;

    float margin_left_scaled;
    float margin_right_scaled;
    float margin_bottom_scaled;

    float key_size_scaled;
    unsigned int key_char_size_scaled;
    float key_border_thickness_scaled;
    float key_spacing_scaled;

    unsigned int key_info_char_size_scaled;
    float key_info_position_offset_scaled;

    float bar_velocity_scaled;

    void CalculateScale(void)
    {
        // TODO: get dpi from windows or linux with wayland or x11.
        dpi_scale = 1.0F;
        dpi_scale_relative = dpi_scale / dpi_scale_reference;
        scale_final = scale * dpi_scale_relative;
    }

    void CalculateDPIScaledValues(void)
    {
        CalculateScale();

        window_height_scaled = window_height * scale_final;

        margin_left_scaled = margin_left * scale_final;
        margin_right_scaled = margin_right * scale_final;
        margin_bottom_scaled = margin_bottom * scale_final;

        key_size_scaled = key_size * scale_final;
        key_char_size_scaled = key_char_size * scale_final;
        key_border_thickness_scaled = key_border_thickness * scale_final;
        key_spacing_scaled = key_spacing * scale_final;

        key_info_char_size_scaled = key_info_char_size * scale_final;
        key_info_position_offset_scaled = key_info_position_offset * scale_final;

        bar_velocity_scaled = bar_velocity * scale_final;
    }
};
static const std::string config_font_name_default = "MiSans-Regular.ttf";
static const sf::Color config_key_info_color_default = sf::Color::White;
static const sf::Color config_bg_color_default = sf::Color::Black;

template<typename T> T LoadOrSetDefaultConfigNode(YAML::Node &node, const std::string &key, const T &default_value, bool *node_inexist_or_valid = nullptr)
{
    if (!node[key])
    {
        if (node_inexist_or_valid != nullptr)
            *node_inexist_or_valid = true;
            
        node[key] = default_value;

        return default_value;
    }
    
    T result;
    try
    {
        result = node[key].as<T>();
    }
    catch (const std::exception &exception)
    {
        std::cerr << "Failed to read value from \"" << key << "\": " << exception.what() << '\n';
        *node_inexist_or_valid = true;

        node[key] = default_value;

        return default_value;
    }

    return result;
}

static sf::Color LoadOrSetDefaultColorConfigNode(YAML::Node &node, const std::string &key, const sf::Color &default_color, bool *node_inexist_or_valid = nullptr)
{
    YAML::Node color_node = node[key];
    std::uint8_t r = LoadOrSetDefaultConfigNode<int>(color_node, "R", default_color.r, node_inexist_or_valid);
    std::uint8_t g = LoadOrSetDefaultConfigNode<int>(color_node, "G", default_color.g, node_inexist_or_valid);
    std::uint8_t b = LoadOrSetDefaultConfigNode<int>(color_node, "B", default_color.b, node_inexist_or_valid);
    std::uint8_t a = LoadOrSetDefaultConfigNode<int>(color_node, "A", default_color.a, node_inexist_or_valid);
    return sf::Color(r, g, b, a);
}

static void LoadOrBuildConfig(const std::string &config_file_name, Config &config)
{
    bool config_write_needed = false;

    YAML::Node config_root;
    
    if (std::filesystem::exists(config_file_name))
        config_root = YAML::LoadFile(config_file_name);
    else
        config_write_needed = true;

    config.framerate_limit = LoadOrSetDefaultConfigNode(config_root, config_framerate_limit_key, Config::framerate_limit_default, &config_write_needed);

    config.dpi_scale_enabled = LoadOrSetDefaultConfigNode(config_root, config_dpi_scale_enabled_key, true, &config_write_needed);
    config.dpi_scale_reference = LoadOrSetDefaultConfigNode(config_root, config_dpi_scale_reference_key, 1.0F, &config_write_needed);

    config.scale = LoadOrSetDefaultConfigNode(config_root, config_scale_key, 1.0F, &config_write_needed);

    config.font_name = LoadOrSetDefaultConfigNode(config_root, config_font_name_key, config_font_name_default, &config_write_needed);

    config.window_height = LoadOrSetDefaultConfigNode(config_root, config_window_height_key, Config::window_height_default, &config_write_needed);
    
    config.margin_left = LoadOrSetDefaultConfigNode(config_root, config_margin_left_key, Config::margin_left_default, &config_write_needed);
    config.margin_right = LoadOrSetDefaultConfigNode(config_root, config_margin_right_key, Config::margin_right_default, &config_write_needed);
    config.margin_bottom = LoadOrSetDefaultConfigNode(config_root, config_margin_bottom_key, Config::margin_bottom_default, &config_write_needed);

    config.bg_color = LoadOrSetDefaultColorConfigNode(config_root, config_bg_color_key, config_bg_color_default, &config_write_needed);

    config.key_size = LoadOrSetDefaultConfigNode(config_root, config_key_size_key, Config::key_size_default, &config_write_needed);
    config.key_border_thickness = LoadOrSetDefaultConfigNode(config_root, config_key_border_thickness_key, Config::key_border_thickness_default, &config_write_needed);
    config.key_char_size = LoadOrSetDefaultConfigNode(config_root, config_key_char_size_key, Config::key_char_size_default, &config_write_needed);
    config.key_spacing = LoadOrSetDefaultConfigNode(config_root, config_key_spacing_key, Config::key_spacing_default, &config_write_needed);

    config.show_counter = LoadOrSetDefaultConfigNode(config_root, config_show_counter_key, Config::show_counter_default, &config_write_needed);

    config.show_key_bpm = LoadOrSetDefaultConfigNode(config_root, config_show_key_bpm_key, Config::show_key_bpm_default, &config_write_needed);

    config.key_info_char_size = LoadOrSetDefaultConfigNode(config_root, config_key_info_char_size_key, Config::key_info_char_size_default, &config_write_needed);
    config.key_info_position_offset = LoadOrSetDefaultConfigNode(config_root, config_key_info_position_offset_key, Config::key_info_position_offset_default, &config_write_needed);
    config.key_info_color = LoadOrSetDefaultColorConfigNode(config_root, config_key_info_color_key, config_key_info_color_default, &config_write_needed);

    config.bar_velocity = LoadOrSetDefaultConfigNode(config_root, config_bar_velocity_key, Config::bar_velocity_default, &config_write_needed);

    config.CalculateDPIScaledValues();

    bool has_valid_key_configuration = true;
    if (config_root[config_keys_key])
    {
        YAML::Node keys_root = config_root[config_keys_key];

        int key_count = 0;
        while (true)
        {
            const std::string key_key = config_key_key_prefix + std::to_string(key_count + 1);

            if (!keys_root[key_key])
                break;

            YAML::Node key_node = keys_root[key_key];
            
            float width_multiplier = LoadOrSetDefaultConfigNode(key_node, config_key_width_multiplier_key, KeyConfig::width_multiplier_default, &config_write_needed);
            
            // scancode
            sf::Keyboard::Scancode scancode;
            std::optional scancode_optional = magic_enum::enum_cast<sf::Keyboard::Scancode>(
                LoadOrSetDefaultConfigNode<std::string>(
                    key_node,
                    config_key_scancode_key,
                    std::string(magic_enum::enum_name(KeyConfig::scancode_default)),
                    &config_write_needed
                )
            );
            if (scancode_optional.has_value())
                scancode = scancode_optional.value();
            else
            {
                scancode = sf::Keyboard::Scancode::Unknown;
                key_node[config_key_scancode_key] = magic_enum::enum_name(scancode);
            }

            // key name
            sf::String key_name = LoadOrSetDefaultConfigNode<std::string>(key_node, config_key_key_name_key, config_key_key_name_default, &config_write_needed);

            YAML::Node key_color_node = key_node[config_key_color_key];
            std::uint8_t key_r = LoadOrSetDefaultConfigNode<int>(key_color_node, "R", 255, &config_write_needed);
            std::uint8_t key_g = LoadOrSetDefaultConfigNode<int>(key_color_node, "G", 255, &config_write_needed);
            std::uint8_t key_b = LoadOrSetDefaultConfigNode<int>(key_color_node, "B", 255, &config_write_needed);
            std::uint8_t key_a = LoadOrSetDefaultConfigNode<int>(key_color_node, "A", 255, &config_write_needed);
            sf::Color color(key_r, key_g, key_b, key_a);

            KeyConfig key_config = KeyConfig(width_multiplier, scancode, key_name, color);
            config.key_configs.push_back(key_config);

            key_count++;
        }

        if (key_count <= 0)
            has_valid_key_configuration = false;
    }
    else
        has_valid_key_configuration = false;

    if (!has_valid_key_configuration)
    {
        YAML::Node keys_root = config_root[config_keys_key];

        int key_num = 1;
        for (const KeyConfig &default_key_config : default_keys)
        {
            const std::string key_key = config_key_key_prefix + std::to_string(key_num);

            YAML::Node key_node = keys_root[key_key];

            key_node[config_key_width_multiplier_key] = default_key_config.width_multiplier;

            key_node[config_key_scancode_key] = std::string(magic_enum::enum_name(default_key_config.scancode));
            key_node[config_key_key_name_key] = (std::string)default_key_config.key_name;

            YAML::Node key_color_node = key_node[config_key_color_key];
            key_color_node["R"] = (int)default_key_config.color.r;
            key_color_node["G"] = (int)default_key_config.color.g;
            key_color_node["B"] = (int)default_key_config.color.b;
            key_color_node["A"] = (int)default_key_config.color.a;

            key_num++;

            // copy default key config
            KeyConfig key_config_default_copy = KeyConfig(default_key_config);
            config.key_configs.push_back(key_config_default_copy);
        }
    }

    if (config_write_needed)
    {
        std::fstream file(config_file_name, file.out | file.trunc);

        if (!file.is_open())
            return;

        file << config_root << '\n';

        file.close();
    }
}

#endif

static void CenterText(sf::Text &text)
{
    sf::FloatRect local_bounds = text.getLocalBounds();
    sf::Vector2f new_origin = local_bounds.position + (local_bounds.size / 2.0F);

    text.setOrigin(new_origin);
}

#if 1 || REGION_MAIN_CLASSES_AND_STRUCTS

class KeyBar
{
protected:
    float velocity;

    float pos_x;
    float width;

    float start_y;
    float end_y;

    sf::RectangleShape rectangle;

    bool key_released = false;

    bool destroy_req = false;

public:
    KeyBar(const sf::Vector2f start_pos, const float width, const float velocity, const sf::Color color)
        : width(width), velocity(velocity)
    {
        this->pos_x = start_pos.x;
        this->end_y = this->start_y = start_pos.y;

        rectangle.setFillColor(color);
    }

    void Release(void)
    {
        key_released = true;
    }

    bool NeedToBeDestroyed(void)
    {
        return destroy_req;
    }

    void Update(const float delta_time)
    {
        float pos_y_delta = velocity * delta_time;

        if (start_y > 0)
            start_y -= pos_y_delta;

        if (key_released)
        {
            if (end_y > 0)
                end_y -= pos_y_delta;
            else
                destroy_req = true;
        }

        rectangle.setPosition({ pos_x, start_y });
        rectangle.setSize({ width, end_y - start_y });
    }

    const sf::RectangleShape &GetDrawable(void)
    {
        return rectangle;
    }
};

struct RectangleHollowFilled
{
public:
    sf::RectangleShape rect_top;
    sf::RectangleShape rect_right;
    sf::RectangleShape rect_bottom;
    sf::RectangleShape rect_left;

    sf::RectangleShape rect_fill;

    sf::Vector2f pos;
    sf::Vector2f size;

    float thickness;

    bool fill_visable = false;

    RectangleHollowFilled(
        const sf::Vector2f pos,
        const sf::Vector2f size,
        const float thickness,
        const sf::Color color
    ) : pos(pos), size(size), thickness(thickness)
    {
        rect_top.setFillColor(color);
        rect_right.setFillColor(color);
        rect_bottom.setFillColor(color);
        rect_left.setFillColor(color);

        sf::Color color_fill(color);
        color_fill.a /= 2;

        rect_fill.setFillColor(color_fill);

        UpdatePositions();
    }

    void UpdatePositions(void)
    {
        sf::Vector2f pos_right(pos.x + size.x - thickness, pos.y);
        sf::Vector2f pos_bottom(pos.x, pos.y + size.y - thickness);
        sf::Vector2f size_top_bottom(size.x, thickness);
        sf::Vector2f size_left_right(thickness, size.y);

        rect_top.setPosition(pos);
        rect_top.setSize(size_top_bottom);
        rect_right.setPosition(pos_right);
        rect_right.setSize(size_left_right);
        rect_bottom.setPosition(pos_bottom);
        rect_bottom.setSize(size_top_bottom);
        rect_left.setPosition(pos);
        rect_left.setSize(size_left_right);
        
        float double_thickness = thickness * 2.0F;
        sf::Vector2f pos_fill(pos.x + thickness, pos.y + thickness);
        sf::Vector2f size_fill(size.x - double_thickness, size.y - double_thickness);

        rect_fill.setPosition(pos_fill);
        rect_fill.setSize(size_fill);
    }

    void Draw(sf::RenderTarget &target)
    {
        if (fill_visable)
            target.draw(rect_fill);

        target.draw(rect_top);
        target.draw(rect_right);
        target.draw(rect_bottom);
        target.draw(rect_left);
    }
};

class KeyWithBar
{
protected:
    sf::Vector2f pos;
    sf::Vector2f size;

    float border_thickness;

    float bar_velocity;

    sf::String key_name;
    const sf::Font &font;
    
    std::vector<KeyBar> key_bars;

    bool key_down = false;
    bool key_down_prev = false;

    sf::Color color;
    sf::Color color_transparent;

    RectangleHollowFilled key_rect;

    sf::Text key_text;
    std::vector<sf::Text> key_info_texts;

    std::string key_info_string;

    bool show_counter;
    bool show_key_bpm;
    float key_info_position_offset;

    int press_count = 0;

    std::chrono::_V2::system_clock::time_point time_point_of_last_key_press;
    float key_press_bpm = 0.0F;

public:
    sf::Keyboard::Scancode scancode;

    KeyWithBar(
        const sf::Vector2f pos,
        const float size,
        const float width_multiplier,
        const float border_thickness,
        const float bar_velocity,
        const sf::Keyboard::Scancode scancode,
        const sf::String &key_name,
        const sf::Font &font,
        const unsigned int key_char_size,
        const unsigned int key_info_char_size,
        const bool show_counter,
        const bool show_key_bpm,
        const float key_info_position_offset,
        const sf::Color key_color,
        const sf::Color key_info_color
    ) :
        pos(pos),
        size({ size * width_multiplier, size }),
        border_thickness(border_thickness),
        bar_velocity(bar_velocity),
        scancode(scancode),
        key_name(key_name),
        font(font),
        color(key_color),
        key_rect(pos, this->size, border_thickness, key_color),
        key_text(font, key_name, key_char_size),
        show_counter(show_counter),
        show_key_bpm(show_key_bpm),
        key_info_position_offset(key_info_position_offset),
        time_point_of_last_key_press(std::chrono::high_resolution_clock::now())
    {
        // transparent color (for pressed effect)

        color_transparent = key_color;
        color_transparent.a /= 2;

        // key name

        CenterText(key_text);
        key_text.setPosition(pos + (this->size / 2.0F));
        key_text.setFillColor(key_color);

        // key info

        int line_count = 0;

        if (show_counter)
            line_count++;

        if (show_key_bpm)
            line_count++;

        InitKeyInfoText(font, key_info_char_size, key_info_color, line_count);
    }

    void InitKeyInfoText(const sf::Font &font, const unsigned int key_info_char_size, const sf::Color key_info_color, const int lines)
    {
        float line_spacing = key_info_char_size * 1.1F;

        float key_info_height = line_spacing * lines;

        sf::Vector2f text_pos =
        {
            pos.x + (size.x / 2.0F),
            pos.y + size.y + key_info_position_offset - (key_info_height / 2.0F)
        };

        for (int i = 0; i < lines; i++)
        {
            key_info_texts.push_back(sf::Text(font, "", key_info_char_size));

            auto &key_info_this_line = key_info_texts[i];

            key_info_this_line.setPosition(text_pos);
            key_info_this_line.setFillColor(key_info_color);

            text_pos.y += line_spacing;
        }
    }

    void UpdateKeyPressBPM(void)
    {
        auto time_point_of_this_key_press = std::chrono::high_resolution_clock::now();

        float time_secs_between_this_and_last_key_press =
            std::chrono::duration_cast<std::chrono::duration<float>>(time_point_of_this_key_press - time_point_of_last_key_press).count();

        time_point_of_last_key_press = time_point_of_this_key_press;

        key_press_bpm = 60.0F / (time_secs_between_this_and_last_key_press * 2.0F);
    }

    void Press(void)
    {
        key_rect.fill_visable = true;

        if (!key_down)
        {
            UpdateKeyPressBPM();

            key_bars.push_back(KeyBar(pos, size.x, bar_velocity, color_transparent));

            press_count++;
        }

        key_down = true;
    }

    void Release(void)
    {
        key_rect.fill_visable = false;

        if (key_down)
        {
            if (key_bars.size() > 0)
                key_bars.back().Release();
        }

        key_down = false;
    }

    void UpdateKeyInfo(void)
    {
        int index = 0;

        if (show_counter)
        {
            auto &counter_text = key_info_texts[index];

            key_info_string.clear();
            key_info_string = key_info_string.append(std::to_string(press_count));
            counter_text.setString(key_info_string);

            CenterText(counter_text);

            index++;
        }
        
        if (show_key_bpm)
        {
            auto &key_bpm_text = key_info_texts[index];

            key_info_string.clear();
            key_info_string = key_info_string.append(std::to_string((int)std::round(key_press_bpm))).append(" BPM");
            key_bpm_text.setString(key_info_string);

            CenterText(key_bpm_text);

            index++;
        }
    }

    void UpdateBars(const float delta_time)
    {
        int index = 0;
        int copy_offset = 0;
        int new_size = key_bars.size();

        while (index < new_size)
        {
            if (copy_offset > 0)
                key_bars[index] = key_bars[index + copy_offset];

            KeyBar &key_bar = key_bars[index];

            if (key_bar.NeedToBeDestroyed())
            {
                new_size--;
                copy_offset++;
                continue;
            }

            key_bar.Update(delta_time);

            index++;
        }

        for (int i = 0; i < copy_offset; i++)
            key_bars.pop_back();
    }

    void Update(const float delta_time)
    {
#if defined(_WIN32) || USE_SFML_KEYPRESSED
        bool key_down_curr = sf::Keyboard::isKeyPressed(scancode);        

        if (!key_down_prev && key_down_curr)
            Press();

        if (key_down_prev && !key_down_curr)
            Release();

        key_down_prev = key_down_curr;
#endif

        UpdateKeyInfo();

        UpdateBars(delta_time);
    }

    void Draw(sf::RenderTarget &target)
    {
        for (KeyBar &key_bar : key_bars)
            target.draw(key_bar.GetDrawable());

        key_rect.Draw(target);

        target.draw(key_text);

        for (const auto &text : key_info_texts)
            target.draw(text);
    }
};

#endif

#if defined(__linux__) && USE_LINUX_SHOWMETHEKEY_CLI

static std::map<sf::Keyboard::Scancode, bool> key_state_change_smtk;
static pthread_mutex_t key_state_change_smtk_mutex;
static bool smtk_poll_thread_exit_req = false;
static bool smtk_poll_thread_exit_flag = false;

struct ShowMeTheKeyProcess
{
    GSubprocess *process = nullptr;
    GDataInputStream *cli_output = nullptr;
};

static void *ThreadPollKeyInput(void *arg)
{
    ShowMeTheKeyProcess *showmethekey_process = (ShowMeTheKeyProcess *)arg;

    if (showmethekey_process->cli_output != nullptr)
    {
        while (true)
        {
            g_autoptr(GError) error = nullptr;
            g_autofree char *line = g_data_input_stream_read_line(
                showmethekey_process->cli_output, nullptr, nullptr, &error
            );

            if (line != nullptr)
            {
                try
                {
                    nlohmann::json json = nlohmann::json::parse(line);

                    int json_key_code = json["key_code"].get<int>();
                    std::string json_key_name = json["key_name"].get<std::string>();
                    std::string json_state_name = json["state_name"].get<std::string>();

                    // std::cout << "key_code: " << json_key_code << "; key_name: " << json_key_name << "; state_name: " << json_state_name << ";\n";

                    pthread_mutex_lock(&key_state_change_smtk_mutex);

                    // TODO: mapping key code to sfml's
                    key_state_change_smtk.insert_or_assign
                    (
                        magic_enum::enum_cast<sf::Keyboard::Scancode>(json_key_name).value_or(sf::Keyboard::Scancode::Unknown),
                        json_state_name == "PRESSED" ? true : false
                    );

                    pthread_mutex_unlock(&key_state_change_smtk_mutex);
                }
                catch (const std::exception &exception)
                {
                    std::cerr << "Failed to parse json string: " << exception.what() << '\n';
                }
            }
            else if (error != nullptr)
                g_warning("Error while reading line: %s.", error->message);
        }

    }

    pthread_exit(nullptr);
    return nullptr;
}

static bool StartShowMeTheKeyCLI(ShowMeTheKeyProcess &showmethekey_process)
{
    GSubprocessFlags showmethekey_process_flags
    (
        GSubprocessFlags(
            G_SUBPROCESS_FLAGS_STDIN_PIPE  |
            G_SUBPROCESS_FLAGS_STDOUT_PIPE |
            G_SUBPROCESS_FLAGS_STDERR_PIPE
        )
    );

    g_autoptr(GError) error = nullptr;

    showmethekey_process.process = g_subprocess_new
    (
        showmethekey_process_flags,
        &error,
        "pkexec", "showmethekey-cli", nullptr
    );

    if (showmethekey_process.process == nullptr)
    {
        std::cerr << "Failed to run showmethekey-cli as subprocess: " << error->message << '\n';
        return false;
    }

    showmethekey_process.cli_output = g_data_input_stream_new(
		g_subprocess_get_stdout_pipe(showmethekey_process.process)
	);

    return true;
}

#endif

int main(int arg_count, char *arg_list[])
{
#if defined(__linux__) && USE_LINUX_SHOWMETHEKEY_CLI

    // showmethekey-cli
    ShowMeTheKeyProcess showmethekey_process;
    
    StartShowMeTheKeyCLI(showmethekey_process);

    pthread_t smtk_polling_process;
    pthread_mutex_init(&key_state_change_smtk_mutex, nullptr);
    pthread_create(&smtk_polling_process, nullptr, ThreadPollKeyInput, (void *)&showmethekey_process);

#endif

    // config
    Config config;

    std::string config_file_name = default_config_file_name;

    if (arg_count >= 2)
        config_file_name = arg_list[1];
    
    LoadOrBuildConfig(config_file_name, config);

    // font
    sf::Font font(config.font_name);

    // drawable keys
    std::vector<KeyWithBar> keys;
    
    float window_width_final = config.margin_left_scaled;

    float key_pos_y = config.window_height_scaled - config.margin_bottom_scaled - config.key_size_scaled;

    for (const KeyConfig &key_config : config.key_configs)
    {
        keys.push_back(
            KeyWithBar(
                { window_width_final, key_pos_y },      // pos
                config.key_size_scaled,                 // size
                key_config.width_multiplier,            // width_multiplier
                config.key_border_thickness_scaled,     // border_thickness
                config.bar_velocity_scaled,             // bar_velocity
                key_config.scancode,                    // scancode
                key_config.key_name,                    // key_name
                font,                                   // font
                config.key_char_size_scaled,            // key_char_size
                config.key_info_char_size_scaled,       // key_info_char_size
                config.show_counter,                    // show_counter
                config.show_key_bpm,                    // show_key_bpm
                config.key_info_position_offset_scaled, // key_info_position_offset
                key_config.color,                       // key_color
                config.key_info_color                   // key_info_color
            )
        );

        float key_width = config.key_size_scaled * key_config.width_multiplier;

        window_width_final += key_width + config.key_spacing_scaled;
    }
    window_width_final += config.margin_right_scaled - config.key_spacing_scaled;

    // sfml window creation
    sf::RenderWindow window
    (
        sf::VideoMode({ (unsigned int)window_width_final, (unsigned int)config.window_height_scaled }),
        window_title,
        sf::Style::Titlebar | sf::Style::Close
    );

    if (config.framerate_limit > 0)
        window.setFramerateLimit(config.framerate_limit);
    else
        window.setVerticalSyncEnabled(true);

    std::map<sf::Keyboard::Scancode, bool> key_state_changes;

    auto time_last = std::chrono::high_resolution_clock::now();
    
    while (true)
    {
        auto time_curr = std::chrono::high_resolution_clock::now();

        float delta_time = std::chrono::duration_cast<std::chrono::duration<float>>(time_curr - time_last).count();

        key_state_changes.clear();

#if defined(__linux__) && USE_LINUX_SHOWMETHEKEY_CLI

        pthread_mutex_lock(&key_state_change_smtk_mutex);

        for (const auto& [scancode, pressed] : key_state_change_smtk)
            key_state_changes.insert_or_assign(scancode, pressed);

        key_state_change_smtk.clear();

        pthread_mutex_unlock(&key_state_change_smtk_mutex);

#endif

        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

#if USE_SFML_KEY_EVENT
            // key input from sfml events
            if (const auto *key_pressed_event = event->getIf<sf::Event::KeyPressed>())
                key_state_changes.insert_or_assign(key_pressed_event->scancode, true);

            if (const auto *key_released_event = event->getIf<sf::Event::KeyReleased>())
                key_state_changes.insert_or_assign(key_released_event->scancode, false);
#endif
        }
        
        if (!window.isOpen())
            break;

        window.clear(config.bg_color);

        for (KeyWithBar &key : keys)
        {
            if (key_state_changes.contains(key.scancode))
            {
                bool key_state_change = key_state_changes[key.scancode];

                if (key_state_change == true)
                    key.Press();
                else
                    key.Release();
            }

            key.Update(delta_time);
            key.Draw(window);
        }

        window.display();

        time_last = time_curr;
    }
    
#if defined(__linux__) && USE_LINUX_SHOWMETHEKEY_CLI

    pthread_cancel(smtk_polling_process);
    
    if (showmethekey_process.process != nullptr)
    {
        const char cmd_stop[] = "stop\n";
		g_subprocess_communicate
        (
			showmethekey_process.process, g_bytes_new(cmd_stop, sizeof(cmd_stop)), NULL, NULL, NULL, NULL
		);
    }

    pthread_mutex_destroy(&key_state_change_smtk_mutex);

#endif

    return 0;
}
