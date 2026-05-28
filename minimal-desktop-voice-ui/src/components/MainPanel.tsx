import {
  Volume2,
  MicOff,
  VolumeX,
  Users,
  Wifi,
} from 'lucide-react';
import { Channel, User } from '../types';

interface MainPanelProps {
  activeChannel: Channel | null;
  currentUser: User;
}

function UserCard({ user, isCurrentUser }: { user: User; isCurrentUser?: boolean }) {
  const isActive = user.isSpeaking && !user.isMuted && !user.isDeafened;

  return (
    <div
      className={`flex items-center gap-2 px-3 py-1.5 rounded-sm
        ${isActive ? 'bg-[#2a3a2a]' : 'hover:bg-[#2e2e2e]'}
        ${isCurrentUser ? 'border border-[#3a4a5a]' : ''}
      `}
    >
      <div className="relative flex items-center justify-center w-5 h-5">
        {isActive && (
          <div className="absolute -inset-1 rounded-sm bg-green-600/15" />
        )}
        <div
          className={`w-3 h-3 rounded-full ${
            isActive
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
        className={`text-[12px] ${
          user.isMuted || user.isDeafened
            ? 'text-gray-500'
            : 'text-gray-200'
        }`}
      >
        {user.name}
        {isCurrentUser && (
          <span className="text-gray-500 ml-1">(You)</span>
        )}
      </span>

      <div className="ml-auto flex items-center gap-1">
        {user.isMuted && (
          <MicOff className="w-3.5 h-3.5 text-red-500" />
        )}
        {user.isDeafened && (
          <VolumeX className="w-3.5 h-3.5 text-amber-500" />
        )}
      </div>
    </div>
  );
}

export default function MainPanel({ activeChannel, currentUser }: MainPanelProps) {
  if (!activeChannel) {
    return (
      <div className="flex-1 bg-[#2b2b2b] flex items-center justify-center">
        <div className="text-center">
          <Volume2 className="w-10 h-10 text-gray-600 mx-auto mb-3" />
          <p className="text-[13px] text-gray-500">
            Select a voice channel to join
          </p>
        </div>
      </div>
    );
  }

  const allUsers = [...activeChannel.users];
  const isCurrentUserInChannel = allUsers.some((u) => u.id === currentUser.id);
  if (!isCurrentUserInChannel) {
    allUsers.push(currentUser);
  }

  return (
    <div className="flex-1 bg-[#2b2b2b] flex flex-col min-w-0">
      {/* Channel header */}
      <div className="flex items-center gap-3 px-4 py-2 border-b border-[#333333]">
        <Volume2 className="w-4 h-4 text-gray-400 flex-shrink-0" />
        <span className="text-[14px] font-semibold text-gray-200">
          {activeChannel.name}
        </span>
        <div className="flex items-center gap-1 ml-auto text-gray-500">
          <Users className="w-3.5 h-3.5" />
          <span className="text-[11px]">{allUsers.length}</span>
        </div>
        <div className="flex items-center gap-1 text-gray-500">
          <Wifi className="w-3.5 h-3.5" />
          <span className="text-[11px]">24ms</span>
        </div>
      </div>

      {/* Users list */}
      <div className="flex-1 overflow-y-auto p-2 space-y-0.5">
        {allUsers.map((user) => (
          <UserCard
            key={user.id}
            user={user}
            isCurrentUser={user.id === currentUser.id}
          />
        ))}
      </div>
    </div>
  );
}
