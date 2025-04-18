前端测试：
```html
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <title>WebSocket Chat Room</title>
    <script src="./sockjs.min.js"></script>
    <script src="./stomp.min.js"></script>
    <script>
      var stompClient = null;

      function connect() {
        var socket = new SockJS("http://localhost:8080/chat");
        stompClient = Stomp.over(socket);
        stompClient.connect({}, function (frame) {
          console.log("Connected: " + frame);
          stompClient.subscribe("/topic/public", function (message) {
            showMessage(JSON.parse(message.body));
          });
        });
      }

      function sendMessage() {
        var message = {
          type: "CHAT",
          content: "Hello, World!", 
          sender: "John", 
          roomId: "room1", 
        };
        stompClient.send("/app/chat.sendMessage", {}, JSON.stringify(message));
        document.getElementById("message").value = "";
      }

      function showMessage(message) {
        var messages = document.getElementById("messages");
        var newMessage = document.createElement("p");
        newMessage.textContent = message;
        messages.appendChild(newMessage);
      }

      window.onload = connect;
    </script>
  </head>
  <body>
    <h1>WebSocket Chat Room</h1>
    <div id="messages"></div>
    <input type="text" id="message" placeholder="Type your message" />
    <button onclick="sendMessage()">Send</button>
  </body>
</html>
```