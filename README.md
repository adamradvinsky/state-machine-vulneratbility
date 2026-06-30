# BLESA Reconnection Vulnerability — Winsock Demo

A C++ demonstration of the **BLESA (Bluetooth Low Energy Spoofing Attack)** vulnerability, originally discovered by Jianliang Wu et al. at Purdue University and published at USENIX WOOT 2020.

This project simulates the core security flaw using TCP sockets (Winsock) to make the concept runnable and observable on any Windows machine — no Bluetooth hardware required.

---

## What is BLESA?

BLESA is a class of spoofing attacks targeting the **reconnection procedure** in Bluetooth Low Energy. When two BLE devices (e.g. a phone and a fitness tracker) reconnect after a signal drop, the spec requires both sides to re-establish encryption using a previously shared Long Term Key (LTK). The vulnerability arises when a device **fails to terminate the connection if that encryption step fails** — allowing an attacker to impersonate a trusted device without ever knowing the real key.

The original paper found this flaw in Linux's BlueZ BLE stack and identified a logic bug in Android and iOS implementations. It affects billions of devices.

> Wu, J., Nan, Y., Kumar, V., Tian, D., Bianchi, A., Payer, M., & Xu, D. (2020).
> *BLESA: Spoofing Attacks against Reconnections in Bluetooth Low Energy.*
> USENIX WOOT '20. https://www.usenix.org/conference/woot20/presentation/wu

---

## How This Demo Works

This project models the BLE reconnection flow using three programs:

### `Server` (the fitness tracker / peripheral)
- Maintains a **bond database** of previously paired clients (name + LTK)
- On each new connection, performs a handshake:
  1. Exchanges device names
  2. Receives the client's claimed LTK
  3. Checks it against the stored bond

### `Client` (the phone / legitimate device)
- Has the correct LTK stored from a prior "pairing" session
- Sends its name and LTK during the reconnection handshake
- Receives an encrypted connection and can send data freely

### `Attacker` (the spoofing device)
- Claims to be the legitimate client by name
- Sends an **incorrect LTK** — simulating a device that has cloned the identity but not the key
- In the **vulnerable** build: the server accepts the connection anyway and proceeds unencrypted
- In the **secure** build: the server detects the mismatch and terminates the connection immediately

---

## The Vulnerability in Code

**Vulnerable behaviour (mirrors the real BLESA bug):**
```cpp
if (checkLTK(result, buffer)) {
    PrivateServerConnectionEncrypted(privateSocket);
} else {
    // BUG: should abort here, but instead silently falls back
    printf("[VULNERABLE] LTK mismatch — continuing unencrypted\n");
    PrivateServerConnectionUnEncrypted(privateSocket);
}
```

**Secure behaviour (the correct fix):**
```cpp
if (checkLTK(result, buffer)) {
    PrivateServerConnectionEncrypted(privateSocket);
} else {
    printf("[SECURE] LTK mismatch — terminating connection\n");
    closesocket(privateSocket);
    return;
}
```

The single missing `closesocket` call is the entire difference between a secure and a vulnerable system.

---

## Build & Run

**Requirements:** Windows, Visual Studio or MinGW, Winsock2 (`Ws2_32.lib`)

```
# Compile (example with g++)
g++ -o main main.cpp Server.cpp Client.cpp -lws2_32

# Run as server (in one terminal)
./main   →  press 'a'

# Run as legitimate client (in another terminal)
./main   →  press 'b'

# Run as attacker (in a third terminal)
./main   →  press 'c'
```

---

## Project Structure

```
├── include/
│   ├── Server.h
│   └── Client.h
├── src/
│   ├── Server.cpp
│   ├── Client.cpp
│   └── main.cpp
└── README.md
```

---

## Key Concepts Demonstrated

| Concept | Real BLE | This Demo |
|---|---|---|
| Bonding / pairing | LTK generated via ECDH | Hardcoded shared integer |
| Bond database | Stored in device firmware | `savedClients` vector |
| Reconnection handshake | Link-layer encryption negotiation | Name + LTK exchange over TCP |
| BLESA vulnerability | No abort on encryption failure | No abort on LTK mismatch |
| Spoofing | Clone MAC address + skip encryption | Clone name + send wrong LTK |
| Fix | Mandatory abort on failure | `closesocket()` on mismatch |

---

## Limitations & Simplifications

- Real BLE uses 128-bit keys derived via Elliptic Curve Diffie-Hellman — this demo uses a plain integer to keep the focus on the logic flaw, not the cryptography
- Real BLESA exploits the radio layer (MAC cloning, jamming) — this demo abstracts that as simply connecting with the same device name
- TCP is reliable and ordered; real BLE link-layer packets can be dropped/replayed in ways TCP prevents

These simplifications are intentional. The goal is to isolate and demonstrate the **logic bug** — the failure to abort on auth failure — which is the actual finding of the paper.

---

## What I Learned

Before building this I thought the interesting part of a security vulnerability was always the cryptography — that attacks meant breaking encryption or guessing a key. What the BLESA paper taught me is that the cryptography in BLE is actually fine. The LTK generation, the key exchange, the encryption itself — none of that was broken. The vulnerability was purely in the **logic of what to do when the crypto fails**, and the answer was just missing a single termination call.

That reframing changed how I think about secure systems. A system isn't secure because it has encryption — it's secure because every failure path is handled correctly. The BLE developers who wrote the Android and Linux stacks almost certainly knew encryption was required. They just didn't explicitly handle the case where it didn't work, and defaulted to continuing instead of aborting. That silent fallback is what turned a well-designed protocol into an exploitable vulnerability across billions of devices.

Building this demo also gave me a concrete understanding of what a **bond database** is and why it exists — it's the mechanism that makes reconnection fast and user-friendly, but it's also the exact thing an attacker exploits by cloning an identity that the database already trusts. The tension between usability (skip re-pairing) and security (verify identity every time) is a real engineering tradeoff, and BLESA is what happens when that tradeoff is made without thinking through all the failure cases.

On the implementation side, this was my first time building a multi-process Winsock application in C++ and managing a handshake protocol from scratch — coordinating send/recv ordering between two programs, handling socket lifecycle correctly, and structuring code so the vulnerable and secure versions are easy to compare directly.

---



- [BLESA Paper (USENIX WOOT 2020)](https://www.usenix.org/conference/woot20/presentation/wu)
- [BlueShield: Detection Counterpart](https://www.usenix.org/conference/raid20/presentation/wu)
- [ZDNet Coverage](https://www.zdnet.com/article/blesa-a-new-bluetooth-vulnerability-impacts-billions-of-devices/)