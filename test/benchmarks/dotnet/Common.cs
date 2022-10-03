using System.Text;

public class Common
{
    public static string GetNumericalFeatures(int feature_size)
    {
        StringBuilder ss = new StringBuilder();
        ss.Append("1:1:0.5 |");
        for (var i = 0; i < feature_size; i++)
        {
            ss.Append(" ");
            ss.Append(i);
            ss.Append(":4.36352");
        }
        return ss.ToString();
    }

    public static string GetStringFeatures(int feature_size)
    {
        StringBuilder ss = new StringBuilder();
        ss.Append("1:1:0.5 | ");
        for (var i = 0; i < feature_size; i++)
        {
            ss.Append("bigfeaturename");
            ss.Append(i);
            ss.Append(":10 ");
        }
        return ss.ToString();
    }

    public static string GetStringFeaturesNoLabel(int feature_size, int action_index=0)
    {
        StringBuilder ss = new StringBuilder();
        ss.Append(" | ");
        for (var i = 0; i < feature_size; i++)
        {
            ss.Append(action_index);
            ss.Append("_");
            ss.Append(i);
            ss.Append(" ");
        }
        ss.Append("\n");
        return ss.ToString();
    }

    public static string GetStringFeaturesMultiEx(int feature_size, int actions, bool shared, bool label, int start_index=0)
    {
        int action_start = 0;
        StringBuilder ss = new StringBuilder();

        if (shared) ss.Append("shared | s_1 s_2 s_3 s_4\n");

        if (label)
        {
            ss.Append("0:1.0:0.5 | ");
            for (var i = 0; i < feature_size; i++)
            {
                ss.Append("0_");
                ss.Append(i);
                ss.Append(" ");
            }
            ss.Append("\n");
            action_start++;
        }

        for (var i = action_start; i < actions; i++)
        {
            ss.Append(" | ");
            for (var j = start_index; j < start_index + feature_size; j++)
            {
                ss.Append(i);
                ss.Append("_");
                ss.Append(" ");
            }
            ss.Append("\n");
        }

        return ss.ToString();
    }
}

