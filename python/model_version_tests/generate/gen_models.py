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

def generate_cb_with_regularization():
    vw = pyvw.Workspace("--cb_explore_adf --l1 0.2 --l2 0.3")

    for _ in range(0,5):
        vw.learn(cb_ex)

    vw.save("cb_explore_adf_model_with_regularization.vw")



def main():
    generate_cb_with_regularization()

if __name__ == "__main__":
    main()

