# Vulnerability Scanner Orchestrator

This project implements an orchestrator that remotely controls multiple vulnerability scanners (Nmap, OWASP ZAP, and Nikto). The orchestrator sends scan commands to remote agents, collects results, and secures communications using SSL/TLS.

Only scan systems you own or have explicit permission to test.

## Architecture and VM Topology

Minimum: 2 VMs. Recommended: 3 VMs.

- VM1 — Orchestrator (Server)
  - Builds and runs the orchestrator at [TCP/orchestrator/server_ssl.c](TCP/orchestrator/server_ssl.c) on TCP port 2222 (configurable).
  - Holds the server certificate and private key from [TCP/cert](TCP/cert).

- VM2 — Agent
  - Builds and runs the agent at [TCP/agent/client_ssl.c](TCP/agent/client_ssl.c).
  - Executes received commands (Nmap, Nikto, etc.) and streams output back to the orchestrator.
  - Must trust the CA certificate in [TCP/cert/ca.crt](TCP/cert/ca.crt).

- VM3 — Target to scan (optional but recommended)
  - A real target or the sample vulnerable HTTP server at [TCP/serveur_test/serveur_vuln.py](TCP/serveur_test/serveur_vuln.py).
  - You can also run the sample HTTP server on VM2 if you prefer a 2-VM setup.

Example network:
- Orchestrator (VM1) at 192.168.100.1:2222
- Agent (VM2) connects to VM1 on 2222
- Target (VM3) reachable from VM2 (e.g., http://TARGET:8080)

## Features

- Orchestration of scanners:
  - Launch scans with Nmap, OWASP ZAP, and Nikto.
  - Configure scan parameters: targets, scan types, custom options.

- Remote agent:
  - Executes scans and arbitrary commands on remote hosts.
  - Returns results securely to the orchestrator.

- Secure communications:
  - TLS via OpenSSL, server authenticated by a local CA.

- Result collection and synthesis:
  - Aggregates and saves results into [TCP/rapport_vulnérabilités.txt](TCP/rapport_vulnérabilités.txt).
  - View report from the orchestrator menu.

## Repository Layout

- Orchestrator: [TCP/orchestrator/server_ssl.c](TCP/orchestrator/server_ssl.c)
- Agent: [TCP/agent/client_ssl.c](TCP/agent/client_ssl.c)
- Certificates: [TCP/cert](TCP/cert)
- Makefile: [TCP/makefile](TCP/makefile)
- Sample vulnerable server: [TCP/serveur_test/serveur_vuln.py](TCP/serveur_test/serveur_vuln.py)
- Aggregated report: [TCP/rapport_vulnérabilités.txt](TCP/rapport_vulnérabilités.txt)

## Prerequisites

On VM1 (orchestrator) and VM2 (agent):
- gcc, Make, OpenSSL dev libraries (e.g., libssl-dev)
- Nmap installed on the agent
- Nikto installed on the agent (Perl + path to nikto.pl)
- Optional: OWASP ZAP on the agent (headless recommended)

Certificates:
- Provided demo CA/server certs in [TCP/cert](TCP/cert). For production, generate your own CA and server certs and deploy:
  - VM1: server.crt and server.key, plus ca.crt
  - VM2: ca.crt to verify the server

## Build

On both VM1 and VM2:
1. Copy this repository (at least the TCP folder) to each VM.
2. In the TCP folder, run:
   - make serv (server)
   - make clt (client)

Note: [TCP/makefile](TCP/makefile) includes convenience targets:
- make run_serv starts the server
- make run_clt starts the client with a hard-coded IP/port; adjust it to your environment or run the binary manually

## Run

- VM1 (Orchestrator):
  - cd TCP
  - make run_serv
  - The server listens on port 2222 by default (see [TCP/orchestrator/server_ssl.c](TCP/orchestrator/server_ssl.c)).

- VM2 (Agent):
  - cd TCP
  - ./client_ssl <SERVER_IP> 2222
  - Example: ./client_ssl 192.168.100.1 2222

- VM3 (Target, optional):
  - To use the sample HTTP server:
    - cd TCP/serveur_test
    - python3 serveur_vuln.py
    - Accessible at http://<TARGET_IP>:8080

## Orchestrator Menu (server side)

When the agent connects, the orchestrator shows:

1. Scan (nmap/nikto/zap)
   - Enter tool name (nmap, nikto, zap), target, and options.
   - The orchestrator builds a command string and sends it to the agent for execution.
   - Note: The Nikto path is hard-coded in [TCP/orchestrator/server_ssl.c](TCP/orchestrator/server_ssl.c) (perl /home/agent1/Bureau/nikto/program/nikto.pl ...). Change this path to match the agent’s installation.
   - Results stream back and are appended to [TCP/rapport_vulnérabilités.txt](TCP/rapport_vulnérabilités.txt).

2. Free shell command
   - Sends any shell command string to the agent; output is streamed back.
   - Use with caution.

3. Scan report synthesis
   - Prints the aggregated report from [TCP/rapport_vulnérabilités.txt](TCP/rapport_vulnérabilités.txt).

4. Quit
   - Closes the SSL session with the agent.

## Usage Examples

- Nmap: options like -sV -p 1-1000
- Nikto: options like -Tuning 9 (ensure nikto path on the agent is correct in the orchestrator)
- ZAP: adjust command template in [TCP/orchestrator/server_ssl.c](TCP/orchestrator/server_ssl.c) if using headless zap.sh

## Security Notes

- TLS:
  - The server presents [TCP/cert/server.crt](TCP/cert/server.crt) signed by [TCP/cert/ca.crt](TCP/cert/ca.crt); the agent verifies it.
- Least privilege:
  - The agent executes commands it receives. Run the agent with minimal privileges and only in trusted networks.
- Reporting:
  - All scan outputs are stored in [TCP/rapport_vulnérabilités.txt](TCP/rapport_vulnérabilités.txt)