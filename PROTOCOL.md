# Protocol
If a message is sent without specifying a channel or user, it will be broadcast
to all channels the user is subscribed to.

-----

### `/subscribe <channel_name> <channel_password>`
Subscribes to the specified channel. Messages from the channel will be
delivered to the user.

### `/unsubscribe <channel_name>`
Unsubscribes from the channel.

### `/list_users <channel_name>`
Lists users subscribed to the specified channel. Only available to channel
subscribers.

### `/delete <channel_name>`
Deletes the specified channel. Only available to channel subscribers.

### `/logout`
Logs out and unauthenticates the connection.

-----

### `#<channel_name> ...message`
Sends a message to the specified channel.

### `@<username> ...message`
Sends a direct message to the specified user.
