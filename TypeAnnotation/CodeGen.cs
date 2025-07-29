using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TypeAnnotation
{
    public class CType
    {
        public string Name = "";
        public object DefaultValue = null;

        public virtual string GetHungarian() { return ""; }
    }

    public enum PrimitiveTypeType
    {
        PT_Bool,
        PT_Byte,
        PT_UByte,
        PT_Int32,
        PT_Int64,
        PT_UInt32,
        PT_UInt64,
        PT_Float,
        PT_Double,
        PT_String
    }

    public class PrimitiveType : CType
    {
        public PrimitiveTypeType Type;

        public override string GetHungarian() { 
            switch (Type)
            {
                case PrimitiveTypeType.PT_Bool: return "b";
                case PrimitiveTypeType.PT_Byte: return "c";
                case PrimitiveTypeType.PT_UByte: return "uc";
                case PrimitiveTypeType.PT_Int32: return "i";
                case PrimitiveTypeType.PT_Int64: return "ll";
                case PrimitiveTypeType.PT_UInt32: return "u";
                case PrimitiveTypeType.PT_UInt64: return "ul";
                case PrimitiveTypeType.PT_Float: return "f";
                case PrimitiveTypeType.PT_Double: return "d";
                case PrimitiveTypeType.PT_String: return "s";
            }
            return "??";
        }
    }

    public class ArrayType : CType
    {
        public CType ElementType;
        public int Size;
    }

    public class StructMember {
        public CType Type;
        public string Name;
        public object DefaultValue = null;
    }

    public class StructType : CType
    {
        public List<StructMember> members_;
        public string baseType_;

        public string ToCode(FormatRules formating, WriteState state) {
            StringBuilder ret = new StringBuilder();

            if (string.IsNullOrWhiteSpace(baseType_))
                ret.Append(state.Indent(string.Format("struct GAME_EXPORT {0} {{\r\n", Name)));
            else
                ret.Append(state.Indent(string.Format("struct GAME_EXPORT {0} : public {1} {{\r\n", Name, baseType_)));
            state.PushScope();

            foreach (var record in members_)
            {
                ret.Append(state.Indent(record.Type.ToString()));
                ret.Append(' ');
                ret.Append(formating.FormatName(record.Type, record.Name));
                if (record.DefaultValue != null)
                    ret.AppendFormat(" = {0}", record.DefaultValue.ToString());
                ret.Append(";\r\n");
            }

            state.PopScope();
            ret.Append(state.Indent("};\r\n"));

            return ret.ToString();
        }
    }

    enum ExternalTypeType
    {
        PT_Entity,
        PT_Component,
        PT_ComponentState
    }

    public class ComponentType : StructType
    {

    }
}
