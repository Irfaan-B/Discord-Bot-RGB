import discord
from discord import app_commands
from discord.ext import commands
from config import TOKEN
import paho.mqtt.client as mqtt
import asyncio

bot = commands.Bot(command_prefix="!", intents = discord.Intents.all())

# MQTT setup
broker_address = "test.mosquitto.org"
mqtt_client = mqtt.Client("discord_bot_golira_epico")

def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")

mqtt_client.on_connect = on_connect

# Connect to the broker
mqtt_client.connect(broker_address)
mqtt_client.loop_start()

@bot.event
async def on_ready():
    print("Bot is Up and Ready!")
    try:
        synced = await bot.tree.sync()
        print(f"Synced {len(synced)} command(s)")
    except Exception as e:
        print(e)



@bot.tree.command(name="mqtt")
@app_commands.describe(topic = "MQTT topic", payload = "MQTT message")
async def mqtt(interaction: discord.Interaction, topic: str, payload: str):
    mqtt_client.publish(topic, payload)
    await interaction.response.send_message(f"Sent `{payload}` to `{topic}`")

@bot.tree.command(name="rgb")
async def rgb(interaction: discord.Interaction, red: int, green: int, blue: int):
    # Invert the values for common anode RGB LED
    inverted_red = 255 - red
    inverted_green = 255 - green
    inverted_blue = 255 - blue
    mqtt_client.publish("home/room/golira", f"RGB{inverted_red},{inverted_green},{inverted_blue}")
    await interaction.response.send_message(f"Set color to RGB({red}, {green}, {blue})")

@bot.tree.command(name="speedup")
async def speedup(interaction: discord.Interaction):
    mqtt_client.publish("home/room/golira", "SPEEDUP")
    await interaction.response.send_message(f"Increased speed")

@bot.tree.command(name="speeddown")
async def speeddown(interaction: discord.Interaction):
    mqtt_client.publish("home/room/golira", "SPEEDDOWN")
    await interaction.response.send_message(f"Decreased speed")

@bot.tree.command(name="breathing")
async def breathing(interaction: discord.Interaction):
    mqtt_client.publish("home/room/golira", "BREATHING")
    await interaction.response.send_message(f"Activated Breathing mode")

@bot.tree.command(name="rainbow")
async def rainbow(interaction: discord.Interaction):
    mqtt_client.publish("home/room/golira", "RAINBOW")
    await interaction.response.send_message(f"Activated Rainbow mode")

@bot.tree.command(name="breathing_off")
async def breathing_off(interaction: discord.Interaction):
    mqtt_client.publish("home/room/golira", "BREATHING_OFF")
    await interaction.response.send_message("Deactivated Breathing mode")

# @bot.tree.command(name="status")
# @app_commands.describe()
# async def status(interaction: discord.Interaction):
#     # The payload "STATUS" is a command for the ESP8266 to send its current status as a MQTT message
#     mqtt_client.publish("home/room/golira", "STATUS")
#     # Wait for the status message to arrive. Timeout after 5 seconds.
#     try:
#         message = await bot.wait_for('mqtt_message', timeout=5.0)
#         await interaction.response.send_message(f"Current status: `{message.payload}`")
#     except asyncio.TimeoutError:
#         await interaction.response.send_message("Timeout while waiting for status update")

# @bot.tree.command(name="speed")
# @app_commands.describe()
# async def speed(interaction: discord.Interaction):
#     # The payload "SPEED" is a command for the ESP8266 to send its current speed as a MQTT message
#     mqtt_client.publish("home/room/golira", "SPEED")
#     # Wait for the speed message to arrive. Timeout after 5 seconds.
#     try:
#         message = await bot.wait_for('mqtt_message', timeout=5.0)
#         await interaction.response.send_message(f"Current speed: `{message.payload}`")
#     except asyncio.TimeoutError:
#         await interaction.response.send_message("Timeout while waiting for speed update")


bot.run(TOKEN)