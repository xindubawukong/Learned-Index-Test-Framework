package algorithms.kiwi;

import java.lang.UnsupportedOperationException;

import java.util.SortedMap;

public class ChunkCell extends Chunk<Cell, Cell>
{
	private static final int DATA_SIZE = 100;		// average # of BYTES of item in data array (guesstimate)
	
	public ChunkCell()
	{
		this(Cell.Empty, null);
	}
	public ChunkCell(Cell minKey, ChunkCell creator)
	{
		super(minKey, DATA_SIZE, creator);
	}
	@Override
	public Chunk<Cell,Cell> newChunk(Cell minKey)
	{
		return new ChunkCell(minKey.clone(), this);
	}
	
	
	@Override
	public Cell readKey(int orderIndex)
	{
		throw new UnsupportedOperationException();
	}
	@Override
	public Object readData(int oi, int di)
	{

		throw new UnsupportedOperationException();
	}

	@Override
	public int copyValues(Object[] result, int idx, int myVer, Cell min, Cell max, SortedMap<Cell, ThreadData.PutData<Cell, Cell>> items) {
		throw new UnsupportedOperationException();
	}

	@Override
	public int allocate(Cell key, Cell data)
	{
		throw new UnsupportedOperationException();
	}

	@Override
	public int allocateSerial(int key, Cell data) {
		throw new UnsupportedOperationException();
	}
}
