package vw;

import java.io.File;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.BlockJUnit4ClassRunner;
import static org.junit.Assert.*;

@RunWith(BlockJUnit4ClassRunner.class)
public final class LdaTest {
  private static final int REPS = 100;

  public LdaTest() {}

  private static class ModelInfo {
    String model;
    String readableModel;
    public ModelInfo(String model, String readableModel) {
      this.model = model;
      this.readableModel = readableModel;
    }
  };

  final static String[] data = new String[] {
    "| 0 2", 
    "| 0 3",
    "| 0 4",
    "| 0 5",
    "| 0 6",
    "| 0 7",
    "| 1 2",
    "| 1 3",
    "| 1 4",
    "| 1 5",
    "| 1 6",
    "| 1 7"
  };

  @Test
  public void testLdaTrainModel() throws Exception {
    trainNewModel();
  }

  @Test
  public void testLdaTrainAndLoadModel() throws Exception {
    assertTrue(rehydratedModelOk(trainNewModel()));
  }

  @Test
  public void testLdaPredict() throws Exception {
    VW v = rehydrateModel(trainNewModel());
    float[] vector = v.multipredict("| 1:1 2:2 3:3");
    assertNotNull(vector);
  }

  private static ModelInfo trainNewModel() throws Exception {
    ModelInfo mi = new ModelInfo(getModelPath(false), getModelPath(true));
    writeVwModelToDisk(mi);
    return mi;
  }

  private static void writeVwModelToDisk(ModelInfo mi) throws Exception {
    final VW vwModel = new VW(String.format("--quiet -b 3 --lda 2 -f %s --readable_model %s",
                                            mi.model, mi.readableModel));

    for (int i = 0; i < REPS; ++i) {
      for (String d : data) {
        vwModel.learn(d);
      }
    }

    vwModel.close();
  }

  /** 
   * Instantiate model to see if it's possible or whether we get an exception.
   * @param model path to binary VW model.
   */
  private static boolean rehydratedModelOk(ModelInfo mi) {
    VW v = null;
    try {
      v = rehydrateModel(mi);
      return true;
    } catch (Throwable e) {
      return false;
    } finally {
      try { 
        v.close(); 
      } catch (Throwable ignored) {}
    }
  }

  private static VW rehydrateModel(ModelInfo mi) {
    return new VW("-i " + mi.model + " -t --quiet");
  }

  private static String getModelPath(final boolean readable) throws Exception {
    File f = File.createTempFile("vwtest", (readable ? ".readable" : "") + ".lda.model");
    String p = f.getAbsolutePath();
    f.deleteOnExit();
    f.delete();
    return p;
  }
}
