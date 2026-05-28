import { useState } from 'react';
import {
  ChevronRight,
  ChevronDown,
  Hash,
  Volume2,
  MicOff,
  VolumeX,
} from 'lucide-react';
import { Channel, User } from '../types';

interface SidebarProps {
  serverName: string;
  channels: Channel[];
  activeChannelId: string | null;
  onChannelClick: (channelId: string) => void;
  collapsed: boolean;
  onToggleCollapse: () => void;
}

function UserRow({ user }: { user: User }) {
  return (
    <div className="flex items-center gap-1.5 px-2 py-0.5 pl-6">
      <div className="relative flex items-center justify-center w-4 h-4">
        {user.isSpeaking && !user.isMuted && !user.isDeafened && (
          <div className="absolute -inset-0.5 rounded-sm bg-green-600/20" />
        )}
        <div
          className={`w-2 h-2 rounded-full ${
            user.isSpeaking && !user.isMuted && !user.isDeafened
              ? 'bg-green-500'
              : user.isMuted
              ? 'bg-red-500'
              : user.isDeafened
              ? 'bg-amber-500'
              : 'bg-gray-500'
          }`}
        />
      </div>
      <span
        className={`text-[11px] truncate ${
          user.isMuted || user.isDeafened ? 'text-gray-500' : 'text-gray-300'
        }`}
      >
        {user.name}
      </span>
      {user.isMuted && (
        <MicOff className="w-3 h-3 text-red-500 flex-shrink-0" />
      )}
      {user.isDeafened && (
        <VolumeX className="w-3 h-3 text-amber-500 flex-shrink-0" />
      )}
    </div>
  );
}

function ChannelRow({
  channel,
  depth,
  activeChannelId,
  onChannelClick,
  expandedIds,
  onToggleExpand,
}: {
  channel: Channel;
  depth: number;
  activeChannelId: string | null;
  onChannelClick: (channelId: string) => void;
  expandedIds: Set<string>;
  onToggleExpand: (id: string) => void;
}) {
  const hasChildren = (channel.children?.length ?? 0) > 0;
  const isExpanded = expandedIds.has(channel.id);
  const isActive = activeChannelId === channel.id;
  const isVoice = channel.type === 'voice';

  return (
    <div>
      <div
        className={`flex items-center gap-1 px-1 py-0.5 cursor-pointer select-none
          ${isActive ? 'bg-[#3a4a5a]' : 'hover:bg-[#333333]'}
        `}
        style={{ paddingLeft: `${4 + depth * 14}px` }}
        onClick={() => {
          if (hasChildren) {
            onToggleExpand(channel.id);
          }
          if (isVoice) {
            onChannelClick(channel.id);
          }
        }}
      >
        {hasChildren ? (
          isExpanded ? (
            <ChevronDown className="w-3 h-3 text-gray-500 flex-shrink-0" />
          ) : (
            <ChevronRight className="w-3 h-3 text-gray-500 flex-shrink-0" />
          )
        ) : (
          <span className="w-3 flex-shrink-0" />
        )}

        {isVoice ? (
          <Volume2 className="w-3.5 h-3.5 text-gray-400 flex-shrink-0" />
        ) : (
          <Hash className="w-3.5 h-3.5 text-gray-500 flex-shrink-0" />
        )}

        <span
          className={`text-[12px] truncate ${
            isActive
              ? 'text-white font-medium'
              : isVoice
              ? 'text-gray-300'
              : 'text-gray-400'
          }`}
        >
          {channel.name}
        </span>

        {isVoice && channel.users.length > 0 && (
          <span className="text-[10px] text-gray-500 ml-auto flex-shrink-0">
            {channel.users.length}
          </span>
        )}
      </div>

      {isExpanded &&
        hasChildren &&
        channel.children?.map((child) => (
          <ChannelRow
            key={child.id}
            channel={child}
            depth={depth + 1}
            activeChannelId={activeChannelId}
            onChannelClick={onChannelClick}
            expandedIds={expandedIds}
            onToggleExpand={onToggleExpand}
          />
        ))}

      {isVoice &&
        channel.users.map((user) => (
          <UserRow key={user.id} user={user} />
        ))}
    </div>
  );
}

export default function Sidebar({
  serverName,
  channels,
  activeChannelId,
  onChannelClick,
  collapsed,
  onToggleCollapse,
}: SidebarProps) {
  const [expandedIds, setExpandedIds] = useState<Set<string>>(
    () => new Set(channels.map((c) => c.id))
  );

  const onToggleExpand = (id: string) => {
    setExpandedIds((prev) => {
      const next = new Set(prev);
      if (next.has(id)) {
        next.delete(id);
      } else {
        next.add(id);
      }
      return next;
    });
  };

  if (collapsed) {
    return (
      <div className="w-8 bg-[#252525] border-r border-[#333333] flex flex-col items-center py-2">
        <button
          onClick={onToggleCollapse}
          className="p-1 hover:bg-[#333333] rounded-sm"
          title="Expand sidebar"
        >
          <ChevronRight className="w-4 h-4 text-gray-400" />
        </button>
      </div>
    );
  }

  return (
    <div className="w-56 bg-[#252525] border-r border-[#333333] flex flex-col flex-shrink-0">
      {/* Server header */}
      <div className="flex items-center gap-2 px-3 py-2 border-b border-[#333333]">
        <button
          onClick={onToggleCollapse}
          className="p-0.5 hover:bg-[#333333] rounded-sm"
          title="Collapse sidebar"
        >
          <ChevronDown className="w-4 h-4 text-gray-400" />
        </button>
        <span className="text-[13px] font-semibold text-gray-200 truncate">
          {serverName}
        </span>
      </div>

      {/* Channel tree */}
      <div className="flex-1 overflow-y-auto py-1">
        {channels.map((channel) => (
          <ChannelRow
            key={channel.id}
            channel={channel}
            depth={0}
            activeChannelId={activeChannelId}
            onChannelClick={onChannelClick}
            expandedIds={expandedIds}
            onToggleExpand={onToggleExpand}
          />
        ))}
      </div>
    </div>
  );
}
