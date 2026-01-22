"""
Flask Application for GC Visualizer
✅ С правильной инициализацией GCSimulator и logs_dir
"""

from flask import Flask, render_template
from flask_cors import CORS
import logging
import os
from config import DevelopmentConfig
from core.gc_simulator import GCSimulator

# Logging configuration
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

# Create Flask app
app = Flask(__name__,
    template_folder='templates',
    static_folder='static',
    static_url_path='/static')

# Load configuration
config = DevelopmentConfig()
app.config.from_object(config)

# Enable CORS
CORS(app)

# Initialize GC Simulator
logger.info(f"Initializing GCSimulator...")

# ✅ ИСПРАВЛЕНО: используем CPP_EXECUTABLE и logs_dir из config
try:
    visualization_dir = os.path.dirname(os.path.abspath(__file__))
    
    # Идем на уровень выше (в GC/)
    gc_dir = os.path.dirname(visualization_dir)
    
    # Теперь берем build/bin/ из GC/
    gc_exe = os.path.join(gc_dir, 'build', 'bin', 'gc_unified.exe')
    
    # Logs в visualization/logs/
    logs_dir = os.path.join(visualization_dir, 'logs')
    
    logger.info(f"GC executable: {gc_exe}")
    logger.info(f"Logs directory: {logs_dir}")
    logger.info(f"Executable exists: {os.path.exists(gc_exe)}")
    
    # ✅ СОЗДАЕМ СИМУЛЯТОР С logs_dir
    gc_simulator = GCSimulator(
        rc_executable=gc_exe,
        ms_executable=gc_exe,
        logs_dir=logs_dir  # ⬅️ ПЕРЕДАЕМ logs_dir ЗДЕСЬ
    )
    
    app.gc_simulator = gc_simulator
    logger.info("✅ GCSimulator initialized successfully")
    
except Exception as e:
    logger.error(f"❌ Failed to initialize GCSimulator: {e}", exc_info=True)
    app.gc_simulator = None

# Register blueprints
from routes.api import api_bp
app.register_blueprint(api_bp, url_prefix='/api')

@app.route('/')
def index():
    """Serve main page"""
    return render_template('index.html')

@app.route('/health')
def health():
    """Health check endpoint"""
    gc_exe = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'build', 'bin', 'gc_unified.exe')
    return {
        'status': 'ok',
        'gc_simulator': 'initialized' if app.gc_simulator else 'failed',
        'gc_binary_exists': os.path.exists(gc_exe),
        'logs_dir_exists': os.path.exists(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'logs'))
    }

if __name__ == '__main__':
    logger.info("Starting Flask server on localhost:5000...")
    app.run(debug=True, host='localhost', port=5000, use_reloader=True)