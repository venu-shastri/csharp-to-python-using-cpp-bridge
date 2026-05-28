namespace EdaModel;

public static class InterfaceDefinitionExtensions
{
    public static Dictionary<int, RegisterDefinition>
        GetAddressMapToRegister(
            this InterfaceDefinition definition)
    {
        return definition.Registers.ToDictionary(
            register => register.Offset,
            register => register);
    }
}
