using System.Collections.Generic;

public interface IJsonExportable
{
    string ExportKey();
    Dictionary<string, object> ExportData();
}