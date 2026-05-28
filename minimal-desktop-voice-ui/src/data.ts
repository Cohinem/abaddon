import { Server, User } from './types';

export const demoUsers: Record<string, User> = {
  u1: { id: 'u1', name: 'Alex', isMuted: false, isDeafened: false, isSpeaking: true },
  u2: { id: 'u2', name: 'Marcus', isMuted: false, isDeafened: false, isSpeaking: false },
  u3: { id: 'u3', name: 'Sarah', isMuted: true, isDeafened: false, isSpeaking: false },
  u4: { id: 'u4', name: 'Jordan', isMuted: false, isDeafened: true, isSpeaking: false },
  u5: { id: 'u5', name: 'Derek', isMuted: false, isDeafened: false, isSpeaking: false },
  u6: { id: 'u6', name: 'Lena', isMuted: true, isDeafened: true, isSpeaking: false },
  u7: { id: 'u7', name: 'Chris', isMuted: false, isDeafened: false, isSpeaking: false },
  u8: { id: 'u8', name: 'Morgan', isMuted: false, isDeafened: false, isSpeaking: true },
  u9: { id: 'u9', name: 'Riley', isMuted: false, isDeafened: false, isSpeaking: false },
  u10: { id: 'u10', name: 'Taylor', isMuted: true, isDeafened: false, isSpeaking: false },
};

export const demoServer: Server = {
  id: 's1',
  name: 'Tactical Gaming Server',
  channels: [
    {
      id: 'c1',
      name: 'General',
      type: 'text',
      users: [],
      children: [
        { id: 'c1-1', name: 'announcements', type: 'text', users: [] },
        { id: 'c1-2', name: 'rules', type: 'text', users: [] },
      ],
    },
    {
      id: 'c2',
      name: 'Squad Alpha',
      type: 'voice',
      users: [demoUsers.u1, demoUsers.u2, demoUsers.u3],
      children: [
        { id: 'c2-1', name: 'Alpha-1', type: 'voice', users: [demoUsers.u4] },
        { id: 'c2-2', name: 'Alpha-2', type: 'voice', users: [] },
      ],
    },
    {
      id: 'c3',
      name: 'Squad Bravo',
      type: 'voice',
      users: [demoUsers.u5, demoUsers.u6],
      children: [
        { id: 'c3-1', name: 'Bravo-1', type: 'voice', users: [demoUsers.u7] },
        { id: 'c3-2', name: 'Bravo-2', type: 'voice', users: [] },
      ],
    },
    {
      id: 'c4',
      name: 'Squad Charlie',
      type: 'voice',
      users: [demoUsers.u8, demoUsers.u9, demoUsers.u10],
      children: [],
    },
    {
      id: 'c5',
      name: 'AFK',
      type: 'voice',
      users: [],
      children: [],
    },
    {
      id: 'c6',
      name: 'Off-Topic',
      type: 'text',
      users: [],
      children: [
        { id: 'c6-1', name: 'music', type: 'text', users: [] },
        { id: 'c6-2', name: 'memes', type: 'text', users: [] },
      ],
    },
  ],
};

export const currentUser: User = {
  id: 'me',
  name: 'Operator',
  isMuted: false,
  isDeafened: false,
  isSpeaking: false,
};
