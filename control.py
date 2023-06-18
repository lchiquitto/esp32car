#!/usr/bin/python3
# vim: ts=4 sw=4 et

from threading import Thread
from tkinter import *
import sys
import os
import json
import signal
import socket

ESP_ADDR = "192.168.100.10"
ESP_PORT = 3333

app = Tk()

tempv = StringVar()
umidv = StringVar()
volumev = StringVar()
distv = StringVar()

def SendUDP(cmd):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.sendto(cmd, (ESP_ADDR, ESP_PORT))

def Forward():
    SendUDP(b'F')
    print("Forward")

def Backward():
    SendUDP(b'B')
    print("Backward")

def Left():
    SendUDP(b'L')
    print("Left")

def Right():
    SendUDP(b'R')
    print("Right")

def Stop():
    SendUDP(b'S')
    print("Stop")

def Read():
    SendUDP(b'X')
    print("Read")

def Quit():
    print("Exiting")
    # XXX: SIGTERM necessário para interromper a thread bloqueada em recvfrom()
    os.kill(os.getpid(), signal.SIGTERM)

def TaskGUI():

    BG = "#037481"

    app.wm_title("A3 - Carro sensor com ESP32")
    app.config(bg = "#828481")

    app.bind("<Up>", lambda event:Forward())
    app.bind("<Down>", lambda event:Backward())
    app.bind("<Left>", lambda event:Left())
    app.bind("<Right>", lambda event:Right())
    app.bind("<Escape>", lambda event:Stop())
    app.bind("q", lambda event:Quit())
    app.bind("x", lambda event:Read())

    cf = Frame(app, width=150, height=300, bg=BG, highlightthickness=2, highlightbackground="#111")
    cf.grid()

    bf = Frame(cf, width=150, height=150, bg=BG)
    bf.grid(row=0, column=0)

    controle = Label(bf, width=12, height=1, text="Controle", font="bold", bg=BG)
    controle.grid(row=0, column=2)

    up = Button(bf, text="FRENTE", command=Forward, bg="green")
    up.grid(row=2, column=2, padx=5, pady=5)

    down = Button(bf, text="TRÁS", command=Backward, bg="yellow")
    down.grid(row=4, column=2, padx=5, pady=5)

    left = Button(bf, text="ESQUERDA", command=Left, bg="orange")
    left.grid(row=3, column=0, padx=5, pady=5)

    right = Button(bf, text="DIREITA", command=Right, bg="blue")
    right.grid(row=3, column=3, padx=5, pady=5)

    stop = Button(bf, text="PARE", command=Stop, bg="red")
    stop.grid(row=3, column=2, padx=5, pady=5)

    read = Button(bf, text="LER", command=Read, bg="white")
    read.grid(row=5, column=0, padx=5, pady=5)

    close = Button(bf, text="SAIR", command=Quit, bg="white")
    close.grid(row=5, column=3, padx=5, pady=5)

    sf = Frame(cf, width=150, height=150, bg=BG)
    sf.grid(row=1, column=0)

    sensores = Label(sf, width=12, height=1, text="Sensores", font="bold", bg=BG)
    sensores.grid(row=0, column=1)
    
    templ = Label(sf, textvariable=tempv, font="bold", bg=BG)
    templ.grid(row=1, column=1)

    umidl = Label(sf, textvariable=umidv, font="bold", bg=BG)
    umidl.grid(row=2, column=1)

    volumel = Label(sf, textvariable=volumev, font="vold", bg=BG)
    volumel.grid(row=3, column=1)

    distl = Label(sf, textvariable=distv, font="vold", bg=BG)
    distl.grid(row=4, column=1)

    tempv.set("0")
    umidv.set("0")
    volumev.set("0 (0)")
    distv.set("0")

    temp = Label(sf, width=12, height=1, text="Temperatura:", font="bold", bg=BG)
    temp.grid(row=1, column=0)

    celsius = Label(sf, width=12, height=1, text="(°C)", font="bold", bg=BG)
    celsius.grid(row=1, column=2)

    umidade = Label(sf, width=12, height=1, text="Umidade:", font="bold", bg=BG)
    umidade.grid(row=2, column=0)

    percent = Label(sf, width=12, height=1, text="(%)", font="bold", bg=BG)
    percent.grid(row=2, column=2)

    volume = Label(sf, width=12, height=1, text="Volume:", font="bold", bg=BG)
    volume.grid(row=3, column=0)

    distancia = Label(sf, width=12, height=1, text="Distância:", font="bold", bg=BG)
    distancia.grid(row=4, column=0)

    cm = Label(sf, width=12, height=1, text="(cm)", font="bold", bg=BG)
    cm.grid(row=4, column=2)

    app.mainloop()

def TaskNET():

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("", ESP_PORT))

    while True:

        data, addr = sock.recvfrom(255)
        try:
            sd = json.loads(data)
        except:
            continue

        tempv.set(sd["T"])
        umidv.set(sd["H"])
        volumev.set("{} ({})".format(sd["SA"], sd["SD"]))
        distv.set(sd["D"])

def main():

    task_net = Thread(target=TaskNET)
    task_net.start()

    TaskGUI()

if __name__ == "__main__":
    main()
