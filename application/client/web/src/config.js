export const SOCKET_TOPIC_RECEIVE = (roomId) => {
  return `/topic/room/${roomId}`
}
export const SOCKET_TOPIC_SEND = (roomId) => {
  return `/app/chat/${roomId}/sendMessage`
}
export const SOCKET_USER_INFO_RECEIVE = (userId) => {
  return `/user/${userId}/info`
}
export const SOCKET_USER_INFO_SEND = (userId) => {
  return `/app/info/${userId}`
}
