import subprocess
# Exported
from subprocess import CalledProcessError
from benchmark.src import logger

from typing import Optional, TextIO

def run(command: str, file: Optional[TextIO] = None):
	ran_cmd = subprocess.run(command.split(), stdout=file if file is not None else subprocess.PIPE) 
	return ran_cmd

def send_email(subject: str, content: str):
	logger.debug("Sending email about failure")
	path = "/Users/glenn/Desktop/mail.zsh"
	subprocess.run(["zsh", path, subject, content])