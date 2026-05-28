import { useState, useEffect, useCallback } from 'react';
import { demoServer, currentUser as initialCurrentUser } from './data';
import { User, Channel } from './types';
import Sidebar from './components/Sidebar';
import MainPanel from './components/MainPanel';
import StatusBar from './components/StatusBar';

function findChannel(channels: Channel[], id: string): Channel | null {
  for (const ch of channels) {
    if (ch.id === id) return ch;
    if (ch.children) {
      const found = findChannel(ch.children, id);
      if (found) return found;
    }
  }
  return null;
}

function getAllVoiceChannels(channels: Channel[]): Channel[] {
  const result: Channel[] = [];
  for (const ch of channels) {
    if (ch.type === 'voice') result.push(ch);
    if (ch.children) {
      result.push(...getAllVoiceChannels(ch.children));
    }
  }
  return result;
}

export default function App() {
  const [activeChannelId, setActiveChannelId] = useState<string | null>('c2');
  const [sidebarCollapsed, setSidebarCollapsed] = useState(false);
  const [currentUser, setCurrentUser] = useState<User>(initialCurrentUser);
  const [channelUsers, setChannelUsers] = useState<Map<string, User[]>>(
    () => {
      const map = new Map<string, User[]>();
      const allVoice = getAllVoiceChannels(demoServer.channels);
      for (const ch of allVoice) {
        map.set(ch.id, [...ch.users]);
      }
      return map;
    }
  );

  const activeChannel = activeChannelId
    ? findChannel(demoServer.channels, activeChannelId)
    : null;

  // Update current user in active channel
  const getActiveChannelUsers = useCallback((): User[] => {
    if (!activeChannelId) return [];
    const baseUsers = channelUsers.get(activeChannelId) ?? [];
    const withoutMe = baseUsers.filter((u) => u.id !== currentUser.id);
    return [...withoutMe, currentUser];
  }, [activeChannelId, channelUsers, currentUser]);

  const handleChannelClick = useCallback(
    (channelId: string) => {
      // Remove current user from old channel
      if (activeChannelId) {
        setChannelUsers((prev) => {
          const next = new Map(prev);
          const oldUsers = next.get(activeChannelId) ?? [];
          next.set(
            activeChannelId,
            oldUsers.filter((u) => u.id !== currentUser.id)
          );
          return next;
        });
      }

      // Add current user to new channel
      setChannelUsers((prev) => {
        const next = new Map(prev);
        const newUsers = next.get(channelId) ?? [];
        if (!newUsers.some((u) => u.id === currentUser.id)) {
          next.set(channelId, [...newUsers, currentUser]);
        }
        return next;
      });

      setActiveChannelId(channelId);
    },
    [activeChannelId, currentUser]
  );

  const handleToggleMute = useCallback(() => {
    setCurrentUser((prev) => ({ ...prev, isMuted: !prev.isMuted }));
  }, []);

  const handleToggleDeafen = useCallback(() => {
    setCurrentUser((prev) => ({
      ...prev,
      isDeafened: !prev.isDeafened,
      isMuted: !prev.isDeafened ? true : prev.isMuted,
    }));
  }, []);

  // Simulate random users speaking
  useEffect(() => {
    const interval = setInterval(() => {
      setChannelUsers((prev) => {
        const next = new Map(prev);
        for (const [chId, users] of next.entries()) {
          const updated = users.map((u) => {
            if (u.id === currentUser.id) return u;
            if (u.isMuted || u.isDeafened) return { ...u, isSpeaking: false };
            // Randomly toggle speaking
            const shouldSpeak = Math.random() < 0.08;
            if (shouldSpeak) {
              return { ...u, isSpeaking: true };
            }
            // Gradually stop speaking
            if (u.isSpeaking && Math.random() < 0.3) {
              return { ...u, isSpeaking: false };
            }
            return u;
          });
          next.set(chId, updated);
        }
        return next;
      });
    }, 500);

    return () => clearInterval(interval);
  }, [currentUser.id]);

  // Build channels with updated users
  const buildChannelsWithUsers = useCallback(
    (channels: Channel[]): Channel[] => {
      return channels.map((ch) => ({
        ...ch,
        users: ch.type === 'voice' ? (channelUsers.get(ch.id) ?? []) : [],
        children: ch.children ? buildChannelsWithUsers(ch.children) : undefined,
      }));
    },
    [channelUsers]
  );

  const channelsWithUsers = buildChannelsWithUsers(demoServer.channels);

  // Build active channel with current user
  const activeChannelWithUser: Channel | null = activeChannel
    ? {
        ...activeChannel,
        users: getActiveChannelUsers(),
      }
    : null;

  return (
    <div className="h-screen w-screen bg-[#1a1a1a] flex flex-col overflow-hidden select-none font-sans">
      {/* Title bar area */}
      <div className="h-7 bg-[#1e1e1e] border-b border-[#333333] flex items-center px-3 flex-shrink-0">
        <span className="text-[11px] text-gray-500 font-medium tracking-wide">
          VoiceChat Client
        </span>
        <span className="text-[11px] text-gray-600 mx-2">|</span>
        <span className="text-[11px] text-gray-400 truncate">
          {demoServer.name}
        </span>
        <span className="text-[11px] text-gray-600 mx-2">|</span>
        <span className="text-[11px] text-gray-500">
          {activeChannel ? activeChannel.name : 'Not connected'}
        </span>
      </div>

      {/* Main content */}
      <div className="flex-1 flex overflow-hidden">
        <Sidebar
          serverName={demoServer.name}
          channels={channelsWithUsers}
          activeChannelId={activeChannelId}
          onChannelClick={handleChannelClick}
          collapsed={sidebarCollapsed}
          onToggleCollapse={() => setSidebarCollapsed((p) => !p)}
        />
        <MainPanel
          activeChannel={activeChannelWithUser}
          currentUser={currentUser}
        />
      </div>

      {/* Status bar */}
      <StatusBar
        username={currentUser.name}
        isMuted={currentUser.isMuted}
        isDeafened={currentUser.isDeafened}
        onToggleMute={handleToggleMute}
        onToggleDeafen={handleToggleDeafen}
      />
    </div>
  );
}
