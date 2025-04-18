package io.github.ttdly.chat.listener;
import org.springframework.context.event.EventListener;
import org.springframework.messaging.simp.stomp.StompHeaderAccessor;
import org.springframework.stereotype.Component;
import org.springframework.web.socket.messaging.SessionConnectedEvent;
import org.springframework.web.socket.messaging.SessionDisconnectEvent;

@Component
public class WebSocketEventListener {
    private final WebSocketConnectionManager connectionManager;

    public WebSocketEventListener(WebSocketConnectionManager connectionManager) {
        this.connectionManager = connectionManager;
    }

    @EventListener
    public void handleWebSocketConnectListener(SessionConnectedEvent event) {
        connectionManager.handleConnectionEstablished(event);
    }

    @EventListener
    public void handleWebSocketDisconnectListener(SessionDisconnectEvent event) {
        StompHeaderAccessor headerAccessor = StompHeaderAccessor.wrap(event.getMessage());
        connectionManager.handleConnectionClosed(event);
    }
}
