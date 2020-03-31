package com.microsoft.sqlserver.javalangextension;

import com.microsoft.sqlserver.javalangextension.AbstractSqlServerExtensionDataset;
import java.lang.IllegalArgumentException;
import java.math.BigDecimal;
import java.sql.Date;
import java.sql.Timestamp;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;

/**
 * Implementation of AbstractSqlServerExtensionDataset that stores
 * simple types as primitives arrays
 */
public class PrimitiveDataset extends AbstractSqlServerExtensionDataset {
	Map<Integer, String>    columnNames;
	Map<Integer, Integer>   columnTypes;
	Map<Integer, Integer>   columnPrecisions;
	Map<Integer, Integer>   columnScales;
	Map<Integer, Object>    columns;
	Map<Integer, boolean[]> columnNullMaps;

	public PrimitiveDataset() {
		columnTypes = new HashMap<>();
		columnNames = new HashMap<>();
		columnPrecisions = new HashMap<>();
		columnScales = new HashMap<>();
		columns = new HashMap<>();
		columnNullMaps = new HashMap<>();
	}

	/**
	 * Column metadata interfaces. Metadata is stored in a hash map, using the column ID as the key
	 */
	public void addColumnMetadata(int columnId, String columnName, int sqlType, int precision, int scale) {
		if (columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " already exists");
		}

		columnTypes.put(columnId, sqlType);
		columnNames.put(columnId, columnName);
		columnPrecisions.put(columnId, precision);
		columnScales.put(columnId, scale);
	}

	public int getColumnCount() {
		return columnTypes.size();
	}

	public String getColumnName(int columnId) {
		checkColumnMetadata(columnId);
		return columnNames.get(columnId);
	}

	public int getColumnPrecision(int columnId) {
		checkColumnMetadata(columnId);
		return columnPrecisions.get(columnId).intValue();
	}

	public int getColumnScale(int columnId) {
		checkColumnMetadata(columnId);
		return columnScales.get(columnId).intValue();
	}

	public int getColumnType(int columnId) {
		checkColumnMetadata(columnId);
		return columnTypes.get(columnId).intValue();
	}

	public boolean[] getColumnNullMap(int columnId) {
		checkColumnMetadata(columnId);
		return columnNullMaps.get(columnId);
	}

	/**
	 * Adding column data interfaces. Column data is stored in a hash map, using the column ID as the key and
	 * an array of the corresponding type representing each row. Primitives cannot be null, thus null values
	 * are represented by an additional boolean array containing a flag to indicate if that value is null.
	 * A null indicator array indicates that there are no null values in that column.
	 */
	public void addIntColumn(int columnId, int[] rows, boolean[] nullMap) {
		checkColumnMetadata(columnId);
		columns.put(columnId, rows);
		columnNullMaps.put(columnId, nullMap);
	}

	public void addBooleanColumn(int columnId, boolean[] rows, boolean[] nullMap) {
		checkColumnMetadata(columnId);
		columns.put(columnId, rows);
		columnNullMaps.put(columnId, nullMap);
	};

	public void addLongColumn(int columnId, long[] rows, boolean[] nullMap) {
		checkColumnMetadata(columnId);
		columns.put(columnId, rows);
		columnNullMaps.put(columnId, nullMap);
	}

	public void addFloatColumn(int columnId, float[] rows, boolean[] nullMap) {
		checkColumnMetadata(columnId);
		columns.put(columnId, rows);
		columnNullMaps.put(columnId, nullMap);
	}

	public void addDoubleColumn(int columnId, double[] rows, boolean[] nullMap) {
		checkColumnMetadata(columnId);
		columns.put(columnId, rows);
		columnNullMaps.put(columnId, nullMap);
	}

	public void addShortColumn(int columnId, short[] rows, boolean[] nullMap) {
		checkColumnMetadata(columnId);
		columns.put(columnId, rows);
		columnNullMaps.put(columnId, nullMap);
	}

	public void addStringColumn(int columnId, String[] rows) {
		checkColumnMetadata(columnId);
		columns.put(columnId, rows);
	}

	public void addBinaryColumn(int columnId, byte[][] rows) {
		checkColumnMetadata(columnId);
		columns.put(columnId, rows);
	}

	public void addDateColumn(int columnId, Date[] rows) {
		checkColumnMetadata(columnId);
		columns.put(columnId, rows);
	}

	public void addNumericColumn(int columnId, BigDecimal[] rows) {
		checkColumnMetadata(columnId);
		columns.put(columnId, rows);
	}

	public void addTimestampColumn(int columnId, Timestamp[] rows) {
		checkColumnMetadata(columnId);
		columns.put(columnId, rows);
	}

	/**
	 * Retreiving column data interfaces. For primitive types, calling getColumnNullMap() for the column ID
	 * will return the boolean array indicating null values.
	 */
	public int[] getIntColumn(int columnId) {
		checkColumnMetadata(columnId);
		return (int[])columns.get(columnId);
	}

	public long[] getLongColumn(int columnId) {
		checkColumnMetadata(columnId);
		return (long[])columns.get(columnId);
	}

	public float[] getFloatColumn(int columnId) {
		checkColumnMetadata(columnId);
		return (float[])columns.get(columnId);
	}

	public short[] getShortColumn(int columnId) {
		checkColumnMetadata(columnId);
		return (short[])columns.get(columnId);
	}

	public boolean[] getBooleanColumn(int columnId) {
		checkColumnMetadata(columnId);
		return (boolean[])columns.get(columnId);
	}

	public double[] getDoubleColumn(int columnId) {
		checkColumnMetadata(columnId);
		return (double[])columns.get(columnId);
	}

	public String[] getStringColumn(int columnId) {
		checkColumnMetadata(columnId);
		return (String[])columns.get(columnId);
	}

	public byte[][] getBinaryColumn(int columnId) {
		checkColumnMetadata(columnId);
		return (byte[][])columns.get(columnId);
	}

	public Date[] getDateColumn(int columnId) {
		checkColumnMetadata(columnId);
		return (Date[])columns.get(columnId);
	}

	public BigDecimal[] getNumericColumn(int columnId) {
		checkColumnMetadata(columnId);
		return (BigDecimal[])columns.get(columnId);
	}

	public Timestamp[] getTimestampColumn(int columnId) {
		checkColumnMetadata(columnId);
		return (Timestamp[])columns.get(columnId);
	}

	private void checkColumnMetadata(int columnId)
	{
		if (!columnTypes.containsKey(columnId)) {
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exist");
		}
	}
}
