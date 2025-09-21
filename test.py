import socket
import time
import unittest
import logging
from datetime import datetime

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s',
    datefmt='%H:%M:%S.%f'
)
logger = logging.getLogger(__name__)

SERVER_HOST = "127.0.0.1"
SERVER_PORT = 8000
PASSWORD = "test"

def send_cmd(sock, cmd, client_name="Unknown"):
    """Send IRC command and flush with logging."""
    logger.info(f"[{client_name}] SENDING: {cmd.strip()}")
    sock.sendall((cmd + "\r\n").encode())

def recv_all(sock, timeout=0.5, client_name="Unknown"):
    """Receive all data available for a short time with logging."""
    sock.setblocking(0)
    total_data = []
    begin = time.time()
    
    logger.debug(f"[{client_name}] Starting to receive data (timeout: {timeout}s)")
    
    while True:
        # Stop after timeout
        if time.time() - begin > timeout:
            break
        try:
            data = sock.recv(4096)
            if data:
                decoded_data = data.decode()
                total_data.append(decoded_data)
                logger.debug(f"[{client_name}] RECEIVED CHUNK: {decoded_data.strip()}")
            else:
                time.sleep(0.01)
        except:
            time.sleep(0.01)
    
    full_response = "".join(total_data)
    if full_response.strip():
        logger.info(f"[{client_name}] FULL RESPONSE: {full_response.strip()}")
    else:
        logger.info(f"[{client_name}] No data received")
    
    return full_response

class TestIRCLogic(unittest.TestCase):
    def setUp(self):
        logger.info("=" * 60)
        logger.info("SETTING UP TEST - Creating and connecting clients")
        logger.info("=" * 60)
        
        # Create sockets for clients
        self.alice = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.bob = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.charlie = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.david = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

        logger.info("Connecting clients to server...")
        self.alice.connect((SERVER_HOST, SERVER_PORT))
        logger.info("Alice connected")
        
        self.bob.connect((SERVER_HOST, SERVER_PORT))
        logger.info("Bob connected")
        
        self.charlie.connect((SERVER_HOST, SERVER_PORT))
        logger.info("Charlie connected")
        
        self.david.connect((SERVER_HOST, SERVER_PORT))
        logger.info("David connected")

        logger.info("Authenticating clients...")
        
        for s, nick, real in [(self.alice, "alice", "Alice"),
                      (self.bob, "bob", "Bob"),
                      (self.charlie, "charlie", "Charlie"),
                      (self.david, "david", "David")]:
            logger.info(f"Authenticating {nick}...")
            
            send_cmd(s, f"PASS {PASSWORD}", nick)
            time.sleep(0.1)
            recv_all(s, client_name=nick)
            
            send_cmd(s, f"NICK {nick}", nick)
            send_cmd(s, f"USER {nick} 0 * :{real}", nick)
            time.sleep(0.1)
            recv_all(s, client_name=nick)
            
            logger.info(f"{nick} authentication complete")

        logger.info("All clients authenticated and ready")

    def tearDown(self):
        logger.info("TEARING DOWN - Closing all connections")
        self.alice.close()
        self.bob.close()
        self.charlie.close()
        self.david.close()
        logger.info("All connections closed")

    def test_operator_quit_server_destroys_channels(self):
        """Test: When operator quits server, all their channels are destroyed"""
        logger.info("=" * 80)
        logger.info("TEST 1: Operator quits server - channels should be destroyed")
        logger.info("=" * 80)
        
        # Alice creates #food and #games (becomes operator of both)
        logger.info("STEP 1: Alice creates #food channel")
        send_cmd(self.alice, "JOIN #food", "alice")
        time.sleep(0.1)
        recv_all(self.alice, client_name="alice")
        
        logger.info("STEP 2: Alice creates #games channel")  
        send_cmd(self.alice, "JOIN #games", "alice")
        time.sleep(0.1)
        recv_all(self.alice, client_name="alice")
        
        # Others join both channels
        logger.info("STEP 3: Bob and Charlie join #food")
        for client, name in [(self.bob, "bob"), (self.charlie, "charlie")]:
            send_cmd(client, "JOIN #food", name)
            time.sleep(0.1)
            recv_all(client, client_name=name)
            
        logger.info("STEP 4: Bob and David join #games")
        for client, name in [(self.bob, "bob"), (self.david, "david")]:
            send_cmd(client, "JOIN #games", name)
            time.sleep(0.1)
            recv_all(client, client_name=name)

        # Clear any residual messages
        time.sleep(0.2)
        for client, name in [(self.bob, "bob"), (self.charlie, "charlie"), (self.david, "david")]:
            recv_all(client, client_name=name)

        # Alice quits server (operator of both channels)
        logger.info("STEP 5: Alice (operator) quits server")
        send_cmd(self.alice, "QUIT :leaving server", "alice")
        time.sleep(0.2)  # Give server time to process

        # Everyone should receive quit notification AND kick from channels
        logger.info("STEP 6: Checking if remaining clients got kicked from channels")
        
        data_bob = recv_all(self.bob, timeout=1.0, client_name="bob")
        data_charlie = recv_all(self.charlie, timeout=1.0, client_name="charlie") 
        data_david = recv_all(self.david, timeout=1.0, client_name="david")

        # Check they received Alice's quit
        logger.info("ASSERTION: Checking quit notifications")
        self.assertIn("alice", data_bob.lower())
        self.assertIn("quit", data_bob.lower())
        logger.info("✓ Bob received Alice's quit notification")

        # Since channels are destroyed, clients should be kicked
        # They might receive PART/KICK messages or the server handles it silently
        
        # Test if channels actually exist by trying to send messages
        logger.info("STEP 7: Testing if channels were destroyed")
        
        logger.info("Bob tries to send message to #food")
        send_cmd(self.bob, "PRIVMSG #food :hello", "bob")
        time.sleep(0.1)
        data_bob = recv_all(self.bob, client_name="bob")
        
        logger.info("Charlie tries to send message to #food")
        send_cmd(self.charlie, "PRIVMSG #food :hi there", "charlie")
        time.sleep(0.1)
        data_charlie = recv_all(self.charlie, client_name="charlie")
        
        logger.info("David tries to send message to #games")
        send_cmd(self.david, "PRIVMSG #games :anyone here?", "david")
        time.sleep(0.1)
        data_david = recv_all(self.david, client_name="david")
        
        # They should get ERR_NOSUCHCHANNEL or similar
        logger.info("ASSERTIONS: Channels should no longer exist")
        if "403" in data_bob or "nosuchchannel" in data_bob.lower():
            logger.info("✓ #food channel was destroyed (Bob got error)")
        else:
            logger.error(f"✗ #food might still exist (Bob response: {data_bob})")
            
        if "403" in data_charlie or "nosuchchannel" in data_charlie.lower():
            logger.info("✓ #food channel was destroyed (Charlie got error)")
        else:
            logger.error(f"✗ #food might still exist (Charlie response: {data_charlie})")
            
        if "403" in data_david or "nosuchchannel" in data_david.lower():
            logger.info("✓ #games channel was destroyed (David got error)")
        else:
            logger.error(f"✗ #games might still exist (David response: {data_david})")

        # Channels should be destroyed
        self.assertTrue("403" in data_bob or "nosuchchannel" in data_bob.lower())
        self.assertTrue("403" in data_charlie or "nosuchchannel" in data_charlie.lower()) 
        self.assertTrue("403" in data_david or "nosuchchannel" in data_david.lower())

    def test_operator_part_destroys_channel(self):
        """Test: When operator leaves channel (PART), channel is destroyed"""
        logger.info("=" * 80)
        logger.info("TEST 2: Operator leaves channel - channel should be destroyed")
        logger.info("=" * 80)
        
        # Alice creates #music, others join
        logger.info("STEP 1: Alice creates #music")
        send_cmd(self.alice, "JOIN #music", "alice")
        time.sleep(0.1)
        recv_all(self.alice, client_name="alice")
        
        logger.info("STEP 2: Bob and Charlie join #music")
        for client, name in [(self.bob, "bob"), (self.charlie, "charlie")]:
            send_cmd(client, "JOIN #music", name)
            time.sleep(0.1)
            recv_all(client, client_name=name)

        # Clear residual messages
        time.sleep(0.2)
        for client, name in [(self.bob, "bob"), (self.charlie, "charlie")]:
            recv_all(client, client_name=name)

        # Alice parts from channel (not quits server, just leaves channel)
        logger.info("STEP 3: Alice (operator) parts from #music")
        send_cmd(self.alice, "PART #music :leaving this channel", "alice")
        time.sleep(0.2)

        # Others should be kicked from channel since it's destroyed
        logger.info("STEP 4: Checking if channel was destroyed")
        data_bob = recv_all(self.bob, timeout=1.0, client_name="bob")
        data_charlie = recv_all(self.charlie, timeout=1.0, client_name="charlie")

        # Test if channel still exists
        logger.info("STEP 5: Bob tries to send message to #music")
        send_cmd(self.bob, "PRIVMSG #music :is anyone here?", "bob")
        time.sleep(0.1)
        data_bob = recv_all(self.bob, client_name="bob")
        
        logger.info("ASSERTION: Channel should be destroyed")
        if "403" in data_bob or "nosuchchannel" in data_bob.lower():
            logger.info("✓ #music channel was destroyed when operator left")
        else:
            logger.error(f"✗ #music still exists (response: {data_bob})")

        self.assertTrue("403" in data_bob or "nosuchchannel" in data_bob.lower())

    def test_channel_recreation_new_operator(self):
        """Test: When channel is recreated, first joiner becomes operator"""
        logger.info("=" * 80)
        logger.info("TEST 3: Channel recreation - first joiner becomes operator") 
        logger.info("=" * 80)
        
        # Alice creates #sports
        logger.info("STEP 1: Alice creates #sports")
        send_cmd(self.alice, "JOIN #sports", "alice")
        time.sleep(0.1)
        recv_all(self.alice, client_name="alice")
        
        # Bob joins
        logger.info("STEP 2: Bob joins #sports")
        send_cmd(self.bob, "JOIN #sports", "bob") 
        time.sleep(0.1)
        recv_all(self.bob, client_name="bob")
        
        # Alice quits (destroys channel)
        logger.info("STEP 3: Alice quits (should destroy #sports)")
        send_cmd(self.alice, "QUIT :bye", "alice")
        time.sleep(0.2)
        recv_all(self.bob, client_name="bob")  # Bob gets quit notification
        
        # Bob tries to rejoin (should recreate channel with Bob as operator)
        logger.info("STEP 4: Bob rejoins #sports (should become operator)")
        send_cmd(self.bob, "JOIN #sports", "bob")
        time.sleep(0.1)
        data_bob = recv_all(self.bob, client_name="bob")
        
        # Check if Bob is operator (should see @bob in channel)
        logger.info("ASSERTION: Bob should be operator in recreated channel")
        if "@bob" in data_bob:
            logger.info("✓ Bob became operator of recreated #sports channel")
        else:
            logger.error(f"✗ Bob is not operator (response: {data_bob})")
            
        self.assertIn("@bob", data_bob)

    def test_client_quit_removes_from_all_channels(self):
        """Test: When client quits, they're removed from all their channels"""
        logger.info("=" * 80)
        logger.info("TEST 4: Client quit removes them from all channels")
        logger.info("=" * 80)
        
        # Create multiple channels with different operators
        logger.info("STEP 1: Alice creates #cooking, Bob creates #coding")
        send_cmd(self.alice, "JOIN #cooking", "alice")  # Alice is operator
        time.sleep(0.1)
        recv_all(self.alice, client_name="alice")
        
        send_cmd(self.bob, "JOIN #coding", "bob")  # Bob is operator  
        time.sleep(0.1)
        recv_all(self.bob, client_name="bob")
        
        # Charlie joins both channels
        logger.info("STEP 2: Charlie joins both channels")
        send_cmd(self.charlie, "JOIN #cooking", "charlie")
        time.sleep(0.1) 
        recv_all(self.charlie, client_name="charlie")
        
        send_cmd(self.charlie, "JOIN #coding", "charlie")
        time.sleep(0.1)
        recv_all(self.charlie, client_name="charlie")
        
        # Clear messages
        time.sleep(0.2)
        for client, name in [(self.alice, "alice"), (self.bob, "bob")]:
            recv_all(client, client_name=name)
        
        # Charlie quits (should be removed from both channels)
        logger.info("STEP 3: Charlie quits server")
        send_cmd(self.charlie, "QUIT :got to go", "charlie")
        time.sleep(0.2)
        
        # Alice and Bob should see Charlie quit from their respective channels
        logger.info("STEP 4: Checking quit notifications in both channels")
        data_alice = recv_all(self.alice, timeout=1.0, client_name="alice")
        data_bob = recv_all(self.bob, timeout=1.0, client_name="bob")
        
        logger.info("ASSERTIONS: Both operators should see Charlie's quit")
        if "charlie" in data_alice.lower() and "quit" in data_alice.lower():
            logger.info("✓ Alice saw Charlie quit from #cooking")
        else:
            logger.error(f"✗ Alice didn't see Charlie quit (response: {data_alice})")
            
        if "charlie" in data_bob.lower() and "quit" in data_bob.lower():
            logger.info("✓ Bob saw Charlie quit from #coding") 
        else:
            logger.error(f"✗ Bob didn't see Charlie quit (response: {data_bob})")
        
        self.assertIn("charlie", data_alice.lower())
        self.assertIn("quit", data_alice.lower())
        self.assertIn("charlie", data_bob.lower()) 
        self.assertIn("quit", data_bob.lower())

if __name__ == "__main__":
    logger.info("Starting Comprehensive IRC Logic Tests")
    unittest.main(verbosity=2)