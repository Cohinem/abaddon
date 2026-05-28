#include "mainwindow.hpp"
#include "abaddon.hpp"
#include "util.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>

MainWindow::MainWindow()
    : m_main_box(Gtk::ORIENTATION_VERTICAL)
    , m_top_bar(Gtk::ORIENTATION_HORIZONTAL)
    , m_middle_area(Gtk::ORIENTATION_HORIZONTAL)
    , m_sidebar(Gtk::ORIENTATION_VERTICAL)
    , m_sidebar_header(Gtk::ORIENTATION_HORIZONTAL)
    , m_main_panel(Gtk::ORIENTATION_VERTICAL)
    , m_placeholder_box(Gtk::ORIENTATION_VERTICAL)
    , m_active_channel_view(Gtk::ORIENTATION_VERTICAL)
    , m_main_panel_header(Gtk::ORIENTATION_HORIZONTAL)
    , m_status_bar(Gtk::ORIENTATION_HORIZONTAL)
    , m_status_user_box(Gtk::ORIENTATION_HORIZONTAL)
    , m_status_avatar(Gtk::ORIENTATION_HORIZONTAL) {
    
    set_default_size(800, 500);
    get_style_context()->add_class("app-window");

    SetupLayout();

    // Connect to Discord Gateway/Voice Signals
    auto &discord = Abaddon::Get().GetDiscordClient();
    
    m_conn_ready = discord.signal_gateway_ready_supplemental().connect(
        sigc::mem_fun(*this, &MainWindow::UpdateUI));
        
    m_conn_voice_state = discord.signal_voice_state_set().connect(
        [this](Snowflake, Snowflake, VoiceStateFlags) { UpdateUI(); });
        
    m_conn_voice_speaking = discord.signal_voice_speaking().connect(
        [this](const VoiceSpeakingData &data) {
            bool is_speaking = (static_cast<int>(data.Speaking) & 1) != 0;
            if (is_speaking) {
                m_speaking_users.insert(data.UserID);
            } else {
                m_speaking_users.erase(data.UserID);
            }
            UpdateUI();
        });
        
    m_conn_voice_connect = discord.signal_voice_connected().connect(
        [this]() { UpdateUI(); });
        
    m_conn_voice_disconnect = discord.signal_voice_disconnected().connect(
        [this]() { UpdateUI(); });
        
    m_conn_user_connect = discord.signal_voice_user_connect().connect(
        [this](Snowflake, Snowflake) { UpdateUI(); });
        
    m_conn_user_disconnect = discord.signal_voice_user_disconnect().connect(
        [this](Snowflake, Snowflake) { UpdateUI(); });

    UpdateUI();
}

MainWindow::~MainWindow() {
    m_conn_ready.disconnect();
    m_conn_voice_state.disconnect();
    m_conn_voice_speaking.disconnect();
    m_conn_voice_connect.disconnect();
    m_conn_voice_disconnect.disconnect();
    m_conn_user_connect.disconnect();
    m_conn_user_disconnect.disconnect();
}

void MainWindow::SetupLayout() {
    // 1. Top Bar
    m_top_bar.set_name("top-bar");
    m_top_bar.set_size_request(-1, 28);
    m_top_bar_title.set_name("top-bar-title");
    m_top_bar_title.set_text("VoiceChat Client");
    m_top_bar_divider1.set_name("top-bar-divider");
    m_top_bar_divider1.set_text(" | ");
    m_top_bar_server.set_name("top-bar-server");
    m_top_bar_server.set_text("Not connected");
    m_top_bar_divider2.set_name("top-bar-divider");
    m_top_bar_divider2.set_text(" | ");
    m_top_bar_channel.set_name("top-bar-channel");
    m_top_bar_channel.set_text("Not connected");

    m_top_bar.pack_start(m_top_bar_title, Gtk::PACK_SHRINK);
    m_top_bar.pack_start(m_top_bar_divider1, Gtk::PACK_SHRINK);
    m_top_bar.pack_start(m_top_bar_server, Gtk::PACK_SHRINK);
    m_top_bar.pack_start(m_top_bar_divider2, Gtk::PACK_SHRINK);
    m_top_bar.pack_start(m_top_bar_channel, Gtk::PACK_SHRINK);

    // 2. Middle Area (Sidebar + Main Panel)
    m_middle_area.set_hexpand(true);
    m_middle_area.set_vexpand(true);

    // 2a. Sidebar (Left)
    m_sidebar.set_name("sidebar");
    m_sidebar.set_size_request(220, -1);
    
    m_sidebar_header.set_name("sidebar-header");
    m_sidebar_header_title.set_name("sidebar-header-title");
    m_sidebar_header_title.set_text("Channels");
    m_sidebar_header.pack_start(m_sidebar_header_title, Gtk::PACK_SHRINK);
    
    m_channel_list_box.set_name("channel-list-box");
    m_channel_list_box.signal_row_activated().connect(
        sigc::mem_fun(*this, &MainWindow::OnChannelRowActivated));
    
    m_channel_scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    m_channel_scroll.add(m_channel_list_box);
    
    m_sidebar.pack_start(m_sidebar_header, Gtk::PACK_SHRINK);
    m_sidebar.pack_start(m_channel_scroll, Gtk::PACK_EXPAND_WIDGET);

    // 2b. Main Panel (Right)
    m_main_panel.set_name("main-panel");
    m_main_panel.set_hexpand(true);
    m_main_panel.set_vexpand(true);

    // Placeholder
    m_placeholder_box.set_name("main-panel-placeholder");
    m_placeholder_box.set_valign(Gtk::ALIGN_CENTER);
    m_placeholder_box.set_halign(Gtk::ALIGN_CENTER);
    m_placeholder_icon.set_from_icon_name("audio-volume-high-symbolic", Gtk::ICON_SIZE_DIALOG);
    m_placeholder_label.set_text("Select a voice channel to join");
    m_placeholder_box.pack_start(m_placeholder_icon, Gtk::PACK_SHRINK);
    m_placeholder_box.pack_start(m_placeholder_label, Gtk::PACK_SHRINK);

    // Active Channel View
    m_active_channel_view.set_hexpand(true);
    m_active_channel_view.set_vexpand(true);
    
    m_main_panel_header.set_name("main-panel-header");
    m_main_panel_icon.set_from_icon_name("audio-volume-high-symbolic", Gtk::ICON_SIZE_MENU);
    m_main_panel_channel_name.set_name("main-panel-channel-name");
    m_main_panel_channel_name.set_text("Channel Name");
    m_main_panel_meta.set_name("main-panel-meta");
    m_main_panel_meta.set_text("0 connected");
    m_main_panel_meta.set_margin_start(12);

    m_main_panel_header.pack_start(m_main_panel_icon, Gtk::PACK_SHRINK);
    m_main_panel_header.pack_start(m_main_panel_channel_name, Gtk::PACK_SHRINK);
    m_main_panel_header.pack_start(m_main_panel_meta, Gtk::PACK_SHRINK);

    m_main_panel_users_box.set_name("main-panel-users-box");
    m_users_scroll.set_policy(Gtk::POLICY_NEVER, Gtk::POLICY_AUTOMATIC);
    m_users_scroll.add(m_main_panel_users_box);

    m_active_channel_view.pack_start(m_main_panel_header, Gtk::PACK_SHRINK);
    m_active_channel_view.pack_start(m_users_scroll, Gtk::PACK_EXPAND_WIDGET);

    m_main_panel.pack_start(m_placeholder_box, Gtk::PACK_EXPAND_WIDGET);
    m_main_panel.pack_start(m_active_channel_view, Gtk::PACK_EXPAND_WIDGET);

    m_middle_area.pack_start(m_sidebar, Gtk::PACK_SHRINK);
    m_middle_area.pack_start(m_main_panel, Gtk::PACK_EXPAND_WIDGET);

    // 3. Status Bar (Bottom)
    m_status_bar.set_name("status-bar");
    m_status_bar.set_size_request(-1, 40);

    // Mute & Deafen Buttons
    Gtk::Image *img_mic = Gtk::manage(new Gtk::Image());
    img_mic->set_from_icon_name("audio-input-microphone-symbolic", Gtk::ICON_SIZE_BUTTON);
    m_btn_mute.set_image(*img_mic);
    m_btn_mute.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::ToggleLocalMute));

    Gtk::Image *img_headphones = Gtk::manage(new Gtk::Image());
    img_headphones->set_from_icon_name("audio-volume-high-symbolic", Gtk::ICON_SIZE_BUTTON);
    m_btn_deafen.set_image(*img_headphones);
    m_btn_deafen.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::ToggleLocalDeafen));

    m_status_divider.set_name("status-bar-divider");

    m_status_connection_icon.set_from_icon_name("network-wireless-symbolic", Gtk::ICON_SIZE_MENU);
    m_status_connection.set_name("status-bar-connection");
    m_status_connection.set_text("Disconnected");
    m_status_latency_divider.set_text(" | ");
    m_status_latency.set_name("status-bar-latency");
    m_status_latency.set_text("24ms");

    // Right Side: User Name and Avatar
    m_status_avatar.set_name("status-bar-avatar");
    m_status_user_box.set_name("status-bar-user-box");
    m_status_username.set_name("status-bar-user-name");
    m_status_username.set_text("Username");

    m_status_user_box.pack_start(m_status_avatar, Gtk::PACK_SHRINK);
    m_status_user_box.pack_start(m_status_username, Gtk::PACK_SHRINK);
    m_status_user_box.set_margin_end(8);

    Gtk::Image *img_settings = Gtk::manage(new Gtk::Image());
    img_settings->set_from_icon_name("preferences-system-symbolic", Gtk::ICON_SIZE_BUTTON);
    m_btn_settings.set_image(*img_settings);
    m_btn_settings.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::OpenSettingsDialog));

    m_status_bar.pack_start(m_btn_mute, Gtk::PACK_SHRINK);
    m_status_bar.pack_start(m_btn_deafen, Gtk::PACK_SHRINK);
    m_status_bar.pack_start(m_status_divider, Gtk::PACK_SHRINK);
    m_status_bar.pack_start(m_status_connection_icon, Gtk::PACK_SHRINK);
    m_status_bar.pack_start(m_status_connection, Gtk::PACK_SHRINK);
    m_status_bar.pack_start(m_status_latency_divider, Gtk::PACK_SHRINK);
    m_status_bar.pack_start(m_status_latency, Gtk::PACK_SHRINK);

    // Spacer to push user info to the right
    Gtk::Box *spacer = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
    m_status_bar.pack_start(*spacer, Gtk::PACK_EXPAND_WIDGET);

    m_status_bar.pack_start(m_status_user_box, Gtk::PACK_SHRINK);
    m_status_bar.pack_start(m_btn_settings, Gtk::PACK_SHRINK);

    // Assemble Main Box
    m_main_box.pack_start(m_top_bar, Gtk::PACK_SHRINK);
    m_main_box.pack_start(m_middle_area, Gtk::PACK_EXPAND_WIDGET);
    m_main_box.pack_start(m_status_bar, Gtk::PACK_SHRINK);

    add(m_main_box);
    show_all();
}

void MainWindow::UpdateUI() {
    UpdateVoiceChannelList();
    UpdateMainPanel();
    UpdateStatusBar();
}

void MainWindow::UpdateVoiceChannelList() {
    // Clear list box
    for (auto *row : m_channel_list_box.get_children()) {
        m_channel_list_box.remove(*row);
        delete row;
    }

    auto &discord = Abaddon::Get().GetDiscordClient();
    if (!discord.IsStarted()) {
        m_sidebar_header_title.set_text("Channels (Not connected)");
        return;
    }

    std::string guild_id_str = Abaddon::Get().GetSettings().GuildID;
    if (guild_id_str.empty()) {
        m_sidebar_header_title.set_text("Set Guild ID in Settings");
        return;
    }

    Snowflake guild_id = Snowflake(guild_id_str);
    auto guild = discord.GetGuild(guild_id);
    if (!guild.has_value()) {
        m_sidebar_header_title.set_text("Guild not found");
        return;
    }

    m_sidebar_header_title.set_text(guild->Name);
    m_top_bar_server.set_text(guild->Name);

    std::set<Snowflake> channel_ids = discord.GetChannelsInGuild(guild_id);
    std::vector<ChannelData> voice_channels;

    for (const auto &ch_id : channel_ids) {
        auto channel = discord.GetChannel(ch_id);
        if (channel.has_value() && (channel->Type == ChannelType::GUILD_VOICE || channel->Type == ChannelType::GUILD_STAGE_VOICE)) {
            voice_channels.push_back(*channel);
        }
    }

    // Sort channels by position
    std::sort(voice_channels.begin(), voice_channels.end(), [](const ChannelData &a, const ChannelData &b) {
        return a.Position.value_or(0) < b.Position.value_or(0);
    });

    Snowflake active_voice_ch = discord.GetVoiceChannelID();

    for (const auto &ch : voice_channels) {
        std::unordered_set<Snowflake> user_ids = discord.GetUsersInVoiceChannel(ch.ID);
        int user_count = user_ids.size();

        // Create Channel row
        ChannelListBoxRow *ch_row = new ChannelListBoxRow(ch.ID, ch.Name.value_or("Voice Channel"), true, user_count);
        if (active_voice_ch.IsValid() && ch.ID == active_voice_ch) {
            ch_row->get_style_context()->add_class("active");
        }
        m_channel_list_box.add(*ch_row);

        // Add user sub-rows
        for (const auto &u_id : user_ids) {
            auto user = discord.GetUser(u_id);
            auto member = discord.GetMember(u_id, guild_id);
            std::string disp_name = user.has_value() ? user->GetUsername() : std::to_string(u_id);
            if (member.has_value() && !member->Nickname.empty()) {
                disp_name = member->Nickname;
            }

            auto vstate = discord.GetVoiceState(u_id);
            bool is_muted = false;
            bool is_deafened = false;
            if (vstate.has_value()) {
                VoiceStateFlags flags = vstate->second.Flags;
                is_muted = util::FlagSet(flags, VoiceStateFlags::Mute) || util::FlagSet(flags, VoiceStateFlags::SelfMute);
                is_deafened = util::FlagSet(flags, VoiceStateFlags::Deaf) || util::FlagSet(flags, VoiceStateFlags::SelfDeaf);
            }

            bool is_speaking = m_speaking_users.find(u_id) != m_speaking_users.end();

            UserListBoxRow *u_row = new UserListBoxRow(u_id, disp_name, is_speaking, is_muted, is_deafened);
            m_channel_list_box.add(*u_row);
        }
    }

    m_channel_list_box.show_all();
}

void MainWindow::UpdateMainPanel() {
    auto &discord = Abaddon::Get().GetDiscordClient();
    Snowflake active_voice_ch = discord.GetVoiceChannelID();

    if (!active_voice_ch.IsValid()) {
        m_placeholder_box.show();
        m_active_channel_view.hide();
        m_top_bar_channel.set_text("Not connected");
        return;
    }

    m_placeholder_box.hide();
    m_active_channel_view.show();

    auto channel = discord.GetChannel(active_voice_ch);
    std::string ch_name = channel.has_value() ? channel->Name.value_or("Voice Channel") : "Voice Channel";
    m_main_panel_channel_name.set_text(ch_name);
    m_top_bar_channel.set_text(ch_name);

    // Get users in channel
    std::unordered_set<Snowflake> user_ids = discord.GetUsersInVoiceChannel(active_voice_ch);
    m_main_panel_meta.set_text(std::to_string(user_ids.size()) + " connected");

    // Clear main panel users ListBox
    for (auto *row : m_main_panel_users_box.get_children()) {
        m_main_panel_users_box.remove(*row);
        delete row;
    }

    Snowflake self_id = discord.GetUserData().ID;
    std::string guild_id_str = Abaddon::Get().GetSettings().GuildID;
    Snowflake guild_id = Snowflake(guild_id_str);

    for (const auto &u_id : user_ids) {
        auto user = discord.GetUser(u_id);
        auto member = discord.GetMember(u_id, guild_id);
        std::string disp_name = user.has_value() ? user->GetUsername() : std::to_string(u_id);
        if (member.has_value() && !member->Nickname.empty()) {
            disp_name = member->Nickname;
        }

        if (u_id == self_id) {
            disp_name += " (You)";
        }

        auto vstate = discord.GetVoiceState(u_id);
        bool is_muted = false;
        bool is_deafened = false;
        if (vstate.has_value()) {
            VoiceStateFlags flags = vstate->second.Flags;
            is_muted = util::FlagSet(flags, VoiceStateFlags::Mute) || util::FlagSet(flags, VoiceStateFlags::SelfMute);
            is_deafened = util::FlagSet(flags, VoiceStateFlags::Deaf) || util::FlagSet(flags, VoiceStateFlags::SelfDeaf);
        }

        bool is_speaking = m_speaking_users.find(u_id) != m_speaking_users.end();

        Gtk::ListBoxRow *card_row = Gtk::manage(new Gtk::ListBoxRow());
        card_row->get_style_context()->add_class("user-card");
        if (is_speaking && !is_muted && !is_deafened) {
            card_row->get_style_context()->add_class("speaking");
        }
        if (u_id == self_id) {
            card_row->get_style_context()->add_class("self");
        }

        Gtk::Box *card_box = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
        
        Gtk::Box *speaking_dot = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL));
        speaking_dot->get_style_context()->add_class("speaking-indicator");
        if (is_speaking && !is_muted && !is_deafened) {
            speaking_dot->get_style_context()->add_class("speaking");
        } else if (is_muted) {
            speaking_dot->get_style_context()->add_class("muted");
        } else if (is_deafened) {
            speaking_dot->get_style_context()->add_class("deafened");
        }
        
        Gtk::Label *name_label = Gtk::manage(new Gtk::Label(disp_name));
        name_label->get_style_context()->add_class("user-card-name");
        if (is_muted || is_deafened) {
            name_label->get_style_context()->add_class("muted");
        }

        card_box->pack_start(*speaking_dot, Gtk::PACK_SHRINK);
        card_box->pack_start(*name_label, Gtk::PACK_SHRINK);
        card_box->set_margin_start(4);

        // Add mute/deafen status icons to the card
        if (is_muted) {
            Gtk::Image *mic_off = Gtk::manage(new Gtk::Image());
            mic_off->set_from_icon_name("microphone-disabled-symbolic", Gtk::ICON_SIZE_MENU);
            card_box->pack_end(*mic_off, Gtk::PACK_SHRINK);
        }
        if (is_deafened) {
            Gtk::Image *headphones_off = Gtk::manage(new Gtk::Image());
            headphones_off->set_from_icon_name("audio-volume-muted-symbolic", Gtk::ICON_SIZE_MENU);
            card_box->pack_end(*headphones_off, Gtk::PACK_SHRINK);
        }

        card_row->add(*card_box);
        m_main_panel_users_box.add(*card_row);
    }

    m_main_panel_users_box.show_all();
}

void MainWindow::UpdateStatusBar() {
    auto &discord = Abaddon::Get().GetDiscordClient();
    if (!discord.IsStarted()) {
        m_status_connection.set_text("Disconnected");
        m_status_connection_icon.set_from_icon_name("network-wireless-offline-symbolic", Gtk::ICON_SIZE_MENU);
        m_status_username.set_text("Not connected");
        m_btn_mute.set_sensitive(false);
        m_btn_deafen.set_sensitive(false);
        return;
    }

    m_btn_mute.set_sensitive(true);
    m_btn_deafen.set_sensitive(true);

    m_status_connection.set_text("Connected");
    m_status_connection_icon.set_from_icon_name("network-wireless-symbolic", Gtk::ICON_SIZE_MENU);
    m_status_username.set_text(discord.GetUserData().GetUsername());

    // Update local mute/deafen buttons style based on state
    Snowflake self_id = discord.GetUserData().ID;
    auto vstate = discord.GetVoiceState(self_id);
    bool is_muted = false;
    bool is_deafened = false;
    if (vstate.has_value()) {
        VoiceStateFlags flags = vstate->second.Flags;
        is_muted = util::FlagSet(flags, VoiceStateFlags::SelfMute) || util::FlagSet(flags, VoiceStateFlags::Mute);
        is_deafened = util::FlagSet(flags, VoiceStateFlags::SelfDeaf) || util::FlagSet(flags, VoiceStateFlags::Deaf);
    }

    if (is_muted) {
        m_btn_mute.get_style_context()->add_class("active-muted");
        Gtk::Image *img = Gtk::manage(new Gtk::Image());
        img->set_from_icon_name("microphone-disabled-symbolic", Gtk::ICON_SIZE_BUTTON);
        m_btn_mute.set_image(*img);
    } else {
        m_btn_mute.get_style_context()->remove_class("active-muted");
        Gtk::Image *img = Gtk::manage(new Gtk::Image());
        img->set_from_icon_name("audio-input-microphone-symbolic", Gtk::ICON_SIZE_BUTTON);
        m_btn_mute.set_image(*img);
    }

    if (is_deafened) {
        m_btn_deafen.get_style_context()->add_class("active-deafened");
        Gtk::Image *img = Gtk::manage(new Gtk::Image());
        img->set_from_icon_name("audio-volume-muted-symbolic", Gtk::ICON_SIZE_BUTTON);
        m_btn_deafen.set_image(*img);
    } else {
        m_btn_deafen.get_style_context()->remove_class("active-deafened");
        Gtk::Image *img = Gtk::manage(new Gtk::Image());
        img->set_from_icon_name("audio-volume-high-symbolic", Gtk::ICON_SIZE_BUTTON);
        m_btn_deafen.set_image(*img);
    }
}

void MainWindow::OnChannelRowActivated(Gtk::ListBoxRow* row) {
    if (!row) return;
    ChannelListBoxRow *ch_row = dynamic_cast<ChannelListBoxRow*>(row);
    if (!ch_row) return;

    auto &discord = Abaddon::Get().GetDiscordClient();
    Snowflake current_ch = discord.GetVoiceChannelID();

    if (ch_row->IsVoiceChannel) {
        if (current_ch.IsValid() && current_ch == ch_row->ID) {
            discord.DisconnectFromVoice();
        } else {
            discord.ConnectToVoice(ch_row->ID);
        }
    }
}

void MainWindow::ToggleLocalMute() {
    auto &discord = Abaddon::Get().GetDiscordClient();
    Snowflake self_id = discord.GetUserData().ID;
    auto vstate = discord.GetVoiceState(self_id);
    bool is_muted = false;
    if (vstate.has_value()) {
        is_muted = util::FlagSet(vstate->second.Flags, VoiceStateFlags::SelfMute);
    }

    is_muted = !is_muted;
    discord.SetVoiceMuted(is_muted);
    Abaddon::Get().GetAudio().SetCapture(!is_muted);
    UpdateUI();
}

void MainWindow::ToggleLocalDeafen() {
    auto &discord = Abaddon::Get().GetDiscordClient();
    Snowflake self_id = discord.GetUserData().ID;
    auto vstate = discord.GetVoiceState(self_id);
    bool is_deafened = false;
    if (vstate.has_value()) {
        is_deafened = util::FlagSet(vstate->second.Flags, VoiceStateFlags::SelfDeaf);
    }

    is_deafened = !is_deafened;
    discord.SetVoiceDeafened(is_deafened);
    Abaddon::Get().GetAudio().SetPlayback(!is_deafened);
    UpdateUI();
}

void MainWindow::OpenSettingsDialog() {
    m_signal_action_set_token.emit();
}

// Stub implementations to preserve compilation compatibility
void MainWindow::UpdateComponents() { UpdateUI(); }
void MainWindow::UpdateMembers() { UpdateUI(); }
void MainWindow::UpdateChannelListing() { UpdateUI(); }
void MainWindow::UpdateChatWindowContents() {}
void MainWindow::UpdateChatActiveChannel(Snowflake id, bool expand_to) {}
Snowflake MainWindow::GetChatActiveChannel() const { return Snowflake::Invalid; }
void MainWindow::UpdateChatNewMessage(const Message &) {}
void MainWindow::UpdateChatMessageDeleted(Snowflake, Snowflake) {}
void MainWindow::UpdateChatMessageUpdated(Snowflake, Snowflake) {}
void MainWindow::UpdateChatPrependHistory(const std::vector<Message> &) {}
void MainWindow::InsertChatInput(const std::string &) {}
Snowflake MainWindow::GetChatOldestListedMessage() { return Snowflake::Invalid; }
void MainWindow::UpdateChatReactionAdd(Snowflake, const Glib::ustring &) {}
void MainWindow::UpdateChatReactionRemove(Snowflake, const Glib::ustring &) {}
void MainWindow::UpdateMenus() {}
void MainWindow::ToggleMenuVisibility() {}
void MainWindow::EditMessage(Snowflake) {}