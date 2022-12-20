import serial
import discord
from dotenv import load_dotenv
import logging
import os
import asyncio

DEBUG = True

__dealer_channel_id__ = 904095864283357214
__seat_channel_ids__ = [904095303551057920,
                        904095941177536513,
                        904095960274206821,
                        904095977248522301
                        ]
__serial_port__ = 'COM5'

#get Discord bot token
load_dotenv()
TOKEN = os.getenv('DISCORD_TOKEN')

client = discord.Client()

@client.event
async def on_ready():
    print(f'{client.user} has connected to Discord!')
    dealer_channel = client.get_channel(__dealer_channel_id__)
    await dealer_channel.send("I'm here!")

@client.event
async def message_dealer(str):
    print("INSIDE MESSAGE_DEALER")
    dealer_channel = client.get_channel(__dealer_channel_id__)
    await dealer_channel.send(str)
    return

@client.event
async def message_seat(str, seat_num):
    voice_channel_id = __seat_channel_ids__[seat_num]
    voice_channel = client.get_channel(voice_channel_id)
    user_id = voice_channel.members[0].id # This works because only one member is allowed in the channel
    user = client.get_user(user_id)

    user.send(str)
    return

@client.event
async def message_all(str):
    message_dealer(str)
    for i in range(len(__seat_channel_ids__)):
        message_seat(str, i)
    return

handler = logging.FileHandler(filename='discord.log', encoding='utf-8', mode='w')

client.run(TOKEN)

# try:
ser = serial.Serial(__serial_port__)
print(ser.name)

if(DEBUG):
    ser.write('Debug test: dealer message')
# except:
#     message = 'Serial port ' + __serial_port__ + ' is not responding.\n' 
#     'Make sure Arduino IDE is not watching the logs'

#     asyncio.run(message_dealer(message))

ser.flushInput()

# while True: #Terminated by keyboard interrupt: Ctrl+C
try:
    ser_line = ser.readline()
    decoded_line = ser_line[0:len(ser_line)-2].decode("utf-8")
    #print(decoded_line)
    line_list = decoded_line.split()

    if not line_list: #if list is empty, line is blank
        continue

    if(line_list[0] == "Seat"):
        #we write to just that seat
        seat_num = int(line_list[1])
        asyncio.run(message_seat(decoded_line, seat_num))

    elif(line_list[0] == "Flop" or line_list[0] == "Turn" or line_list[0] == "River"):
        #we write to all chats
        asyncio.run(message_all(decoded_line))
    else: #We'll write almost everything to the dealer file for debugging
        asyncio.run(message_dealer(decoded_line))
except BaseException as err:
    asyncio.run(message_dealer(err))