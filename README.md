# Discord Bot Golira
A repository for the Discord bot I'm currently developing.

This bot takes Discord slash commands and publishes it to an MQTT topic.
The ESP8266 microcontroller then subscribes to the same topic and receives those messages, after which it will drive the RGB LEDs.
