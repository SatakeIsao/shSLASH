using UnityEngine;
using System.Collections.Generic;

public abstract class JsonExportableMonoBehaviour : MonoBehaviour, IJsonExportable
{
    public virtual string ExportKey()
    {
        return this.GetType().Name;
    }

    public abstract Dictionary<string, object> ExportData();
}