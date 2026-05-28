#pragma once
#include "discord/discord.hpp"
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/listbox.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/image.h>
#include <gtkmm/separator.h>
#include <sigc++/sigc++.h>
#include <unordered_set>
#include <set>
#include <string>

// Custom row for Channel ListBox
class ChannelListBoxRow : public Gtk::ListBoxRow {
public:
    Snowflake ID;
    bool IsVoiceChannel;
    Gtk::Box Box;
    Gtk::Label Label;
    Gtk::Label CountLabel;
    Gtk::Image Icon;

    ChannelListBoxRow(Snowflake id, std::string name, bool is_voice, int user_count)
        : ID(id), IsVoiceChannel(is_voice), Box(Gtk::ORIENTATION_HORIZONTAL) {
        
        get_style_context()->add_class("channel-row");
        
        if (is_voice) {
            Icon.set_from_icon_name("audio-volume-high-symbolic", Gtk::ICON_SIZE_MENU);
        } else {
            Icon.set_from_icon_name("folder-symbolic", Gtk::ICON_SIZE_MENU);
        }
        
        Label.set_text(name);
        Label.get_style_context()->add_class("channel-name");
        Label.set_margin_start(6);
        Label.set_ellipsize(Pango::ELLIPSIZE_END);
        
        Box.pack_start(Icon, Gtk::PACK_SHRINK);
        Box.pack_start(Label, Gtk::PACK_EXPAND_WIDGET);
        
        if (is_voice && user_count > 0) {
            CountLabel.set_text(std::to_string(user_count));
            CountLabel.get_style_context()->add_class("channel-user-count");
            Box.pack_end(CountLabel, Gtk::PACK_SHRINK);
        }
        
        add(Box);
        show_all();
    }
};

// Custom row for User in Channel
class UserListBoxRow : public Gtk::ListBoxRow {
public:
    Snowflake ID;
    Gtk::Box Box;
    Gtk::Box SpeakingIndicator;
    Gtk::Label Label;
    Gtk::Image MuteIcon;
    Gtk::Image DeafenIcon;

    UserListBoxRow(Snowflake id, std::string name, bool is_speaking, bool is_muted, bool is_deafened)
        : ID(id), Box(Gtk::ORIENTATION_HORIZONTAL) {
        
        get_style_context()->add_class("user-row");
        
        SpeakingIndicator.get_style_context()->add_class("speaking-indicator");
        if (is_speaking && !is_muted && !is_deafened) {
            SpeakingIndicator.get_style_context()->add_class("speaking");
        } else if (is_muted) {
            SpeakingIndicator.get_style_context()->add_class("muted");
        } else if (is_deafened) {
            SpeakingIndicator.get_style_context()->add_class("deafened");
        }
        
        Label.set_text(name);
        Label.get_style_context()->add_class("user-name");
        if (is_muted || is_deafened) {
            Label.get_style_context()->add_class("muted");
        }
        Label.set_ellipsize(Pango::ELLIPSIZE_END);
        
        Box.pack_start(SpeakingIndicator, Gtk::PACK_SHRINK);
        Box.pack_start(Label, Gtk::PACK_EXPAND_WIDGET);
        
        if (is_muted) {
            MuteIcon.set_from_icon_name("microphone-disabled-symbolic", Gtk::ICON_SIZE_MENU);
            Box.pack_end(MuteIcon, Gtk::PACK_SHRINK);
        }
        if (is_deafened) {
            DeafenIcon.set_from_icon_name("audio-volume-muted-symbolic", Gtk::ICON_SIZE_MENU);
            Box.pack_end(DeafenIcon, Gtk::PACK_SHRINK);
        }
        
        add(Box);
        show_all();
    }
};

class MainWindow : public Gtk::Window {
public:
    MainWindow();
    virtual ~MainWindow();

    // Interface methods required by abaddon.cpp (stubs or implementations)
    void UpdateComponents();
    void UpdateMembers();
    void UpdateChannelListing();
    void UpdateChatWindowContents();
    void UpdateChatActiveChannel(Snowflake id, bool expand_to);
    Snowflake GetChatActiveChannel() const;
    void UpdateChatNewMessage(const Message &data);
    void UpdateChatMessageDeleted(Snowflake id, Snowflake channel_id);
    void UpdateChatMessageUpdated(Snowflake id, Snowflake channel_id);
    void UpdateChatPrependHistory(const std::vector<Message> &msgs);
    void InsertChatInput(const std::string &text);
    Snowflake GetChatOldestListedMessage();
    void UpdateChatReactionAdd(Snowflake id, const Glib::ustring &param);
    void UpdateChatReactionRemove(Snowflake id, const Glib::ustring &param);
    void UpdateMenus();
    void ToggleMenuVisibility();
    void EditMessage(Snowflake message_id);

    // Dummy getters to prevent build issues
    void* GetChannelList() { return nullptr; }
    void* GetChatWindow() { return nullptr; }
    void* GetMemberList() { return nullptr; }

    // Signals for actions
    typedef sigc::signal<void> type_signal_action_connect;
    typedef sigc::signal<void> type_signal_action_disconnect;
    typedef sigc::signal<void> type_signal_action_set_token;
    typedef sigc::signal<void> type_signal_action_login_qr;
    typedef sigc::signal<void> type_signal_action_reload_css;
    typedef sigc::signal<void> type_signal_action_set_status;
    typedef sigc::signal<void, Snowflake> type_signal_action_add_recipient;
    typedef sigc::signal<void, Snowflake> type_signal_action_view_pins;
    typedef sigc::signal<void, Snowflake> type_signal_action_view_threads;

    type_signal_action_connect signal_action_connect() { return m_signal_action_connect; }
    type_signal_action_disconnect signal_action_disconnect() { return m_signal_action_disconnect; }
    type_signal_action_set_token signal_action_set_token() { return m_signal_action_set_token; }
    type_signal_action_login_qr signal_action_login_qr() { return m_signal_action_login_qr; }
    type_signal_action_reload_css signal_action_reload_css() { return m_signal_action_reload_css; }
    type_signal_action_set_status signal_action_set_status() { return m_signal_action_set_status; }
    type_signal_action_add_recipient signal_action_add_recipient() { return m_signal_action_add_recipient; }
    type_signal_action_view_pins signal_action_view_pins() { return m_signal_action_view_pins; }
    type_signal_action_view_threads signal_action_view_threads() { return m_signal_action_view_threads; }

private:
    void SetupLayout();
    void UpdateUI();
    void UpdateVoiceChannelList();
    void UpdateMainPanel();
    void UpdateStatusBar();
    void OnChannelRowActivated(Gtk::ListBoxRow* row);
    void ToggleLocalMute();
    void ToggleLocalDeafen();
    void OpenSettingsDialog();

    // Signal connections from Discord client
    sigc::connection m_conn_ready;
    sigc::connection m_conn_disconnected;
    sigc::connection m_conn_voice_state;
    sigc::connection m_conn_voice_speaking;
    sigc::connection m_conn_voice_connect;
    sigc::connection m_conn_voice_disconnect;
    sigc::connection m_conn_user_connect;
    sigc::connection m_conn_user_disconnect;

    // Track speaking users
    std::unordered_set<Snowflake> m_speaking_users;
    bool m_is_ready;

    // UI Widgets
    Gtk::Box m_main_box;
    
    // Top Bar
    Gtk::Box m_top_bar;
    Gtk::Label m_top_bar_title;
    Gtk::Label m_top_bar_divider1;
    Gtk::Label m_top_bar_server;
    Gtk::Label m_top_bar_divider2;
    Gtk::Label m_top_bar_channel;

    // Middle Area
    Gtk::Box m_middle_area;

    // Sidebar (Left)
    Gtk::Box m_sidebar;
    Gtk::Box m_sidebar_header;
    Gtk::Label m_sidebar_header_title;
    Gtk::ScrolledWindow m_channel_scroll;
    Gtk::ListBox m_channel_list_box;

    // Main Panel (Right)
    Gtk::Box m_main_panel;
    
    // Main Panel Placeholder
    Gtk::Box m_placeholder_box;
    Gtk::Image m_placeholder_icon;
    Gtk::Label m_placeholder_label;

    // Main Panel Active Channel View
    Gtk::Box m_active_channel_view;
    Gtk::Box m_main_panel_header;
    Gtk::Image m_main_panel_icon;
    Gtk::Label m_main_panel_channel_name;
    Gtk::Label m_main_panel_meta;
    Gtk::ScrolledWindow m_users_scroll;
    Gtk::ListBox m_main_panel_users_box;

    // Status Bar (Bottom)
    Gtk::Box m_status_bar;
    Gtk::Button m_btn_mute;
    Gtk::Button m_btn_deafen;
    Gtk::Separator m_status_divider;
    Gtk::Image m_status_connection_icon;
    Gtk::Label m_status_connection;
    Gtk::Label m_status_latency_divider;
    Gtk::Label m_status_latency;
    Gtk::Box m_status_user_box;
    Gtk::Box m_status_avatar;
    Gtk::Label m_status_username;
    Gtk::Button m_btn_settings;

    // Actions signals
    type_signal_action_connect m_signal_action_connect;
    type_signal_action_disconnect m_signal_action_disconnect;
    type_signal_action_set_token m_signal_action_set_token;
    type_signal_action_login_qr m_signal_action_login_qr;
    type_signal_action_reload_css m_signal_action_reload_css;
    type_signal_action_set_status m_signal_action_set_status;
    type_signal_action_add_recipient m_signal_action_add_recipient;
    type_signal_action_view_pins m_signal_action_view_pins;
    type_signal_action_view_threads m_signal_action_view_threads;
};
