package io.github.ttdly.chat.listener;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.messaging.simp.SimpMessageSendingOperations;
import org.springframework.messaging.simp.stomp.StompHeaderAccessor;
import org.springframework.stereotype.Service;
import org.springframework.web.socket.messaging.SessionConnectedEvent;
import org.springframework.web.socket.messaging.SessionDisconnectEvent;

import java.util.HashSet;
import java.util.Set;

@Service
public class WebSocketConnectionManager {

    @Autowired
    private SimpMessageSendingOperations messagingTemplate;
    private final Set<String> connectedSessions = new HashSet<>();


    public void handleConnectionEstablished(SessionConnectedEvent event) {
        StompHeaderAccessor headers = StompHeaderAccessor.wrap(event.getMessage());
        String sessionId = headers.getSessionId();
        connectedSessions.add(sessionId);
        if (sessionId != null) {
            messagingTemplate.convertAndSendToUser(sessionId, "/queue/notification", "Welcome!");
        }
    }

    public void handleConnectionClosed(SessionDisconnectEvent event) {
        String sessionId = event.getSessionId();
        connectedSessions.remove(sessionId);
    }

    public int getConnectedSessionCount() {
        return connectedSessions.size();
    }
}
