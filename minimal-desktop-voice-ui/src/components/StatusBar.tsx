import {
  Mic,
  MicOff,
  Headphones,
  VolumeX,
  Wifi,
  Settings,
  User,
} from 'lucide-react';

interface StatusBarProps {
  username: string;
  isMuted: boolean;
  isDeafened: boolean;
  onToggleMute: () => void;
  onToggleDeafen: () => void;
}

export default function StatusBar({
  username,
  isMuted,
  isDeafened,
  onToggleMute,
  onToggleDeafen,
}: StatusBarProps) {
  return (
    <div className="h-10 bg-[#1e1e1e] border-t border-[#333333] flex items-center px-2 gap-1 flex-shrink-0">
      {/* Mute button */}
      <button
        onClick={onToggleMute}
        className={`flex items-center justify-center w-8 h-8 rounded-sm transition-colors
          ${
            isMuted
              ? 'bg-red-900/40 text-red-400 hover:bg-red-900/60'
              : 'text-gray-400 hover:bg-[#333333] hover:text-gray-200'
          }
        `}
        title={isMuted ? 'Unmute' : 'Mute'}
      >
        {isMuted ? (
          <MicOff className="w-4 h-4" />
        ) : (
          <Mic className="w-4 h-4" />
        )}
      </button>

      {/* Deafen button */}
      <button
        onClick={onToggleDeafen}
        className={`flex items-center justify-center w-8 h-8 rounded-sm transition-colors
          ${
            isDeafened
              ? 'bg-amber-900/40 text-amber-400 hover:bg-amber-900/60'
              : 'text-gray-400 hover:bg-[#333333] hover:text-gray-200'
          }
        `}
        title={isDeafened ? 'Undeafen' : 'Deafen'}
      >
        {isDeafened ? (
          <VolumeX className="w-4 h-4" />
        ) : (
          <Headphones className="w-4 h-4" />
        )}
      </button>

      {/* Divider */}
      <div className="w-px h-5 bg-[#333333] mx-1" />

      {/* Connection status */}
      <div className="flex items-center gap-1.5 px-2">
        <Wifi className="w-3.5 h-3.5 text-green-500" />
        <span className="text-[11px] text-gray-400">Connected</span>
        <span className="text-[10px] text-gray-600">|</span>
        <span className="text-[11px] text-gray-500">24ms</span>
      </div>

      {/* Spacer */}
      <div className="flex-1" />

      {/* User info */}
      <div className="flex items-center gap-2 px-2">
        <div className="w-6 h-6 rounded-sm bg-[#3a4a5a] flex items-center justify-center">
          <User className="w-3.5 h-3.5 text-gray-300" />
        </div>
        <span className="text-[12px] text-gray-300">{username}</span>
      </div>

      {/* Settings */}
      <button
        className="flex items-center justify-center w-8 h-8 rounded-sm text-gray-400 hover:bg-[#333333] hover:text-gray-200 transition-colors"
        title="Settings"
      >
        <Settings className="w-4 h-4" />
      </button>
    </div>
  );
}
