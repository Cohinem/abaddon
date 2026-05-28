export interface User {
  id: string;
  name: string;
  avatar?: string;
  isMuted: boolean;
  isDeafened: boolean;
  isSpeaking: boolean;
}

export interface Channel {
  id: string;
  name: string;
  type: 'text' | 'voice';
  users: User[];
  children?: Channel[];
}

export interface Server {
  id: string;
  name: string;
  icon?: string;
  channels: Channel[];
}
