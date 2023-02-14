import subprocess
# Exported
from subprocess import CalledProcessError
from benchmark.src import logger

from typing import Optional, TextIO

def run(command: str, shell: bool = False):
	ran_cmd = subprocess.run(command.split(), stdout=subprocess.PIPE, shell=shell) 
	return ran_cmd

def popen(command: str, file: TextIO):
	proc = subprocess.Popen(command.split(), stdout=file)
	return proc

def send_email(subject: str, content: str):
	logger.debug("Sending email about failure")
	path = "/Users/glenn/Desktop/mail.zsh"
	subprocess.run(["zsh", path, subject, content])