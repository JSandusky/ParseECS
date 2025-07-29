using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TypeAnnotation
{
    public class FormatRules
    {
        public bool AppendUnderscore { get; set; }
        public bool PrependUnderscore { get; set; }
        public bool CamelCase { get; set; }
        public bool StartCapital { get; set; }
        public bool UseHungarian { get; set; }
        public bool UseLightHungarian { get; set; }

        static string neither = "{0}";
        static string appendOnly = "{0}_";
        static string prependOnly = "_{0}";
        static string both = "_{0}_" ;

        public string FormatName(CType type, string name)
        {
            string formatString = AppendUnderscore && PrependUnderscore ? both : (AppendUnderscore ? appendOnly : (PrependUnderscore ? prependOnly : neither));
            if (UseLightHungarian)
                formatString = type.GetHungarian() + formatString;
            else if (UseHungarian)
                formatString = "m_" + type.GetHungarian() + formatString;

            return string.Format(formatString, name);
        }
    }

    public class WriteState
    {
        public int IndentDepth { get; set; }

        public void PushScope() { ++IndentDepth; }
        public void PopScope() { --IndentDepth; }

        public string Indent(string input)
        {
            string ret = "";
            for (int i = 0; i < IndentDepth; ++i)
                ret += "    ";
            return ret + input;
        }
    }
}
