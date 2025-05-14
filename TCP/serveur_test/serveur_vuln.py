from http.server import HTTPServer, BaseHTTPRequestHandler

class SimpleVulnerableHTTPHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        # Simuler une page vulnérable avec un en-tête suspect
        self.send_response(200)
        self.send_header('Server', 'Apache/2.2.8 (Ubuntu) PHP/5.2.4-2ubuntu5')  # volontairement ancien
        self.send_header('X-Powered-By', 'PHP/5.2.4')  # souvent détecté comme vulnérable
        self.end_headers()

        if self.path == "/phpinfo.php":
            content = "<html><body><h1>phpinfo()</h1><p>Simulé</p></body></html>"
        else:
            content = "<html><body><h1>Bienvenue sur un serveur test</h1></body></html>"

        self.wfile.write(content.encode())

    def log_message(self, format, *args):
        # Supprimer les logs dans la console pour plus de clarté
        return

if __name__ == "__main__":
    PORT = 8080
    server = HTTPServer(('0.0.0.0', PORT), SimpleVulnerableHTTPHandler)
    print(f"[*] Serveur HTTP vulnérable en écoute sur le port {PORT}...")
    print("[*] Accédez à http://localhost:8080 ou scannez avec Nikto.")
    server.serve_forever()
