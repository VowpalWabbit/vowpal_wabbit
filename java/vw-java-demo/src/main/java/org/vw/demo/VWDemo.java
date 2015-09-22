
package org.vw.demo;

import java.util.*;
import vw.VW;

public class VWDemo {

  public static void main(String ... args) throws Exception {
    if ( args.length != 1) {
      System.err.println("usage: VWDemo <VW_DATA_DIR>");
      System.exit(1);
    }
    String vwDataDir = args[0];
    String cmd = String.format("-i %s/predictor.vw -t --quiet", vwDataDir);
    System.out.println("cmd: " + cmd);
    VW vw = new VW(cmd);
    System.out.println("version: " + vw.version());
    float[] vector = vw.getTopicPredictions("| 142:5 9:1 28:40");
    List<Float> predictions = new ArrayList<>();
    for (Float w : vector)
      predictions.add(w);
    System.out.println("predictions: " + predictions);
  }

}
