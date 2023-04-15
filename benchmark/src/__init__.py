from logging import getLogger, basicConfig, DEBUG
basicConfig(
	format='%(asctime)s %(levelname)-8s %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S',
	level=DEBUG
)

logger = getLogger("benchmark-logger")