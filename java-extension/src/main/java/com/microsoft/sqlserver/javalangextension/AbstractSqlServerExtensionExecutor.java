package com.microsoft.sqlserver.javalangextension;

import com.microsoft.sqlserver.javalangextension.AbstractSqlServerExtensionDataset;
import java.lang.UnsupportedOperationException;
import java.util.LinkedHashMap;

/**
 * Abstract class containing interface used by the Java extension
 */
public abstract class AbstractSqlServerExtensionExecutor {
	/* Supported versions of the Java extension */
	public final int SQLSERVER_JAVA_LANG_EXTENSION_V1 = 1;

	/* Members used by the extension to determine application specifics */
	protected int executorExtensionVersion;
	protected String executorInputDatasetClassName;
	protected String executorOutputDatasetClassName;

	public AbstractSqlServerExtensionExecutor() { }

	public void init(String sessionId, int taskId, int numTasks) {
		/* Default implementation of init() is no-op */
	}

	public AbstractSqlServerExtensionDataset execute(AbstractSqlServerExtensionDataset input, LinkedHashMap<String, Object> params) {
		throw new UnsupportedOperationException("AbstractSqlServerExtensionExecutor execute() is not implemented");
	}

	public void cleanup() {
		/* Default implementation of cleanup() is no-op */
	}
}