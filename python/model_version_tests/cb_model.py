import argparse
import vowpalwabbit as pyvw


cb_ex = """
    shared |User b
    |Action d
    |Action e
    |Action f
    |Action ff
    4:1:0.2 |Action fff
    """

def generate():
    vw = pyvw.Workspace("--cb_explore_adf --l1 0.2 --l2 0.3")

    for _ in range(0,5):
        vw.learn(cb_ex)

    vw.save("cb_explore_adf_model_with_regularization.vw")


def load(model_path):
    from vowpalwabbit import pyvw
    vw = pyvw.vw(f"-i {model_path}/cb_explore_adf_model_with_regularization.vw")
    for _ in range(0,5):
        vw.predict(cb_ex)
        vw.learn(cb_ex)



def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--model_path",
        type=str,
        default="vw_generated_models"
    )
    parser.add_argument(
        "--generate",
        action="store_true",
    )
    parser.add_argument(
        "--load",
        action="store_true",
    )
    args = parser.parse_args()

    if args.generate:
        generate()

    elif args.load:
        load(args.model_path)
    else:
        raise RuntimeError("specify either --generate or --load")

if __name__ == "__main__":
    main()

