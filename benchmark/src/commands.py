import subprocess
from subprocess import CalledProcessError


def run(command: str):
	ran_cmd = subprocess.run(command.split(), stdout=subprocess.PIPE) 
	return ran_cmd

def send_email(subject: str, content: str):
	path = "/Users/glenn/Desktop/mail.zsh"
	subprocess.run(["zsh", path, subject, content])