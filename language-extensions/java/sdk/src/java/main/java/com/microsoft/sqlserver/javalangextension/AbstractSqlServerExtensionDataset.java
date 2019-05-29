package com.microsoft.sqlserver.javalangextension;

import java.lang.UnsupportedOperationException;
import java.sql.Date;

/**
 * Abstract class containing interface for handling input and output data used by the Java
 * extension.
 */
public class AbstractSqlServerExtensionDataset {
	/**
	 * Column metadata interfaces
	 */
	public void addColumnMetadata(int columnId, String columnName, int columnType, int precision, int scale) {
		throw new UnsupportedOperationException("addColumnMetadata is not implemented");
	}

	public int getColumnCount() {
		throw new UnsupportedOperationException("getColumnCount is not implemented");
	}

	public String getColumnName(int columnId) {
		throw new UnsupportedOperationException("getColumnName is not implemented");
	}

	public int getColumnPrecision(int columnId) {
		throw new UnsupportedOperationException("getColumnPrecision is not implemented");
	}

	public int getColumnScale(int columnId) {
		throw new UnsupportedOperationException("getColumnScale is not implemented");
	}

	public int getColumnType(int columnId) {
		throw new UnsupportedOperationException("getColumnNullMap is not implemented");
	}

	public boolean[] getColumnNullMap(int columnId) {
		throw new UnsupportedOperationException("getColumnNullMap is not implemented");
	}

	/**
	 * Adding column interfaces
	 */
	public void addIntColumn(int columnId, int[] rows, boolean[] nullMap) {
		throw new UnsupportedOperationException("addIntColumn is not implemented");
	}

	public void addBooleanColumn(int columnId, boolean[] rows, boolean[] nullMap) {
		throw new UnsupportedOperationException("addBooleanColumn is not implemented");
	}

	public void addLongColumn(int columnId, long[] rows, boolean[] nullMap) {
		throw new UnsupportedOperationException("addLongColumn is not implemented");
	}

	public void addFloatColumn(int columnId, float[] rows, boolean[] nullMap) {
		throw new UnsupportedOperationException("addFloatColumn is not implemented");
	}

	public void addDoubleColumn(int columnId, double[] rows, boolean[] nullMap) {
		throw new UnsupportedOperationException("addDoubleColumn is not implemented");
	}

	public void addShortColumn(int columnId, short[] rows, boolean[] nullMap) {
		throw new UnsupportedOperationException("addShortColumn is not implemented");
	}

	public void addStringColumn(int columnId, String[] rows) {
		throw new UnsupportedOperationException("addStringColumn is not implemented");
	}

	public void addBinaryColumn(int columnId, byte[][] rows) {
		throw new UnsupportedOperationException("addBinaryColumn is not implemented");
	}

	public void addDateColumn(int columnId, Date[] rows) {
		throw new UnsupportedOperationException("addDateColumn is not implemented");
	}

	/**
	 * Retrieving column interfaces
	 */
	public int[] getIntColumn(int columnId) {
		throw new UnsupportedOperationException("getIntColumn is not implemented");
	}

	public long[] getLongColumn(int columnId) {
		throw new UnsupportedOperationException("getLongColumn is not implemented");
	}

	public float[] getFloatColumn(int columnId) {
		throw new UnsupportedOperationException("getFloatColumn is not implemented");
	}

	public short[] getShortColumn(int columnId) {
		throw new UnsupportedOperationException("getShortColumn is not implemented");
	}

	public boolean[] getBooleanColumn(int columnId) {
		throw new UnsupportedOperationException("getBooleanColumn is not implemented");
	}

	public double[] getDoubleColumn(int columnId) {
		throw new UnsupportedOperationException("getDoubleColumn is not implemented");
	}

	public String[] getStringColumn(int columnId) {
		throw new UnsupportedOperationException("getStringColumn is not implemented");
	}

	public byte[][] getBinaryColumn(int columnId) {
		throw new UnsupportedOperationException("getBinaryColumn is not implemented");
	}

	public Date[] getDateColumn(int columnId) {
		throw new UnsupportedOperationException("getDateColumn is not implemented");
	}
}