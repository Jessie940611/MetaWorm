import argparse

parser = argparse.ArgumentParser(description="simulate chemotaxis network")

parser.add_argument("--config",
                    help="watch which neuron")

parser.add_argument("--randseed", 
                    type=int,
                    default=0,
                    help="random seed")
