package com.microsoft.sqlserver.javalangextension;

import com.microsoft.sqlserver.javalangextension.AbstractSqlServerExtensionDataset;
import java.lang.IllegalArgumentException;
import java.util.HashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.sql.Date;

/**
 * Implementation of AbstractSqlServerExtensionDataset that stores
 * simple types as primitives arrays
 */
public class PrimitiveDataset extends AbstractSqlServerExtensionDataset {
	Map<Integer, String>  columnNames;
	Map<Integer, Integer> columnTypes;
	Map<Integer, Integer> columnPrecisions;
	Map<Integer, Integer> columnScales;
	Map<Integer, Object>  columns;
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
		if (!columnNames.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		return columnNames.get(columnId);
	}

	public int getColumnPrecision(int columnId) {
		if (!columnPrecisions.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		return columnPrecisions.get(columnId).intValue();
	}

	public int getColumnScale(int columnId) {
		if (!columnScales.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		return columnScales.get(columnId).intValue();
	}

	public int getColumnType(int columnId) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		return columnTypes.get(columnId).intValue();
	}

	public boolean[] getColumnNullMap(int columnId) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		return columnNullMaps.get(columnId);
	}

	/**
	 * Adding column data interfaces. Column data is stored in a hash map, using the column ID as the key and
	 * an array of the corresponding type representing each row. Primitives cannot be null, thus null values
	 * are represented by an additional boolean array containing a flag to indicate if that value is null.
	 * A null indicator array indicates that there are no null values in that column.
	 */
	public void addIntColumn(int columnId, int[] rows, boolean[] nullMap) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		columns.put(columnId, rows);
		columnNullMaps.put(columnId, nullMap);
	}

	public void addBooleanColumn(int columnId, boolean[] rows, boolean[] nullMap) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		columns.put(columnId, rows);
		columnNullMaps.put(columnId, nullMap);
	};

	public void addLongColumn(int columnId, long[] rows, boolean[] nullMap) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		columns.put(columnId, rows);
		columnNullMaps.put(columnId, nullMap);
	}

	public void addFloatColumn(int columnId, float[] rows, boolean[] nullMap) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		columns.put(columnId, rows);
		columnNullMaps.put(columnId, nullMap);
	}

	public void addDoubleColumn(int columnId, double[] rows, boolean[] nullMap) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		columns.put(columnId, rows);
		columnNullMaps.put(columnId, nullMap);
	}

	public void addShortColumn(int columnId, short[] rows, boolean[] nullMap) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		columns.put(columnId, rows);
		columnNullMaps.put(columnId, nullMap);
	}

	public void addStringColumn(int columnId, String[] rows) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		columns.put(columnId, rows);
	}

	public void addBinaryColumn(int columnId, byte[][] rows) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		columns.put(columnId, rows);
	}

	public void addDateColumn(int columnId, Date[] rows) {
		columns.put(columnId, rows);
	}

	/**
	 * Retreiving column data interfaces. For primitive types, calling getColumnNullMap() for the column ID
	 * will return the boolean array indicating null values.
	 */
	public int[] getIntColumn(int columnId) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		return (int[])columns.get(columnId);
	}

	public long[] getLongColumn(int columnId) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		return (long[])columns.get(columnId);
	}

	public float[] getFloatColumn(int columnId) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		return (float[])columns.get(columnId);
	}

	public short[] getShortColumn(int columnId) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		return (short[])columns.get(columnId);
	}

	public boolean[] getBooleanColumn(int columnId) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		return (boolean[])columns.get(columnId);
	}

	public double[] getDoubleColumn(int columnId) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		return (double[])columns.get(columnId);
	}

	public String[] getStringColumn(int columnId) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		return (String[])columns.get(columnId);
	}

	public byte[][] getBinaryColumn(int columnId) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		return (byte[][])columns.get(columnId);
	}

	public Date[] getDateColumn(int columnId) {
		if (!columnTypes.containsKey(columnId))
		{
			throw new IllegalArgumentException("Metadata for column ID #: " + columnId + " does not exists");
		}

		return (Date[])columns.get(columnId);
	}
}
