package de.hpi.mocc.jujo.exercise4;

import org.apache.flink.api.common.functions.MapFunction;
import org.apache.flink.api.common.functions.ReduceFunction;
import org.apache.flink.api.common.functions.RichMapFunction;
import org.apache.flink.api.java.DataSet;
import org.apache.flink.api.java.ExecutionEnvironment;
import org.apache.flink.api.java.functions.FunctionAnnotation;
import org.apache.flink.api.java.operators.IterativeDataSet;
import org.apache.flink.api.java.tuple.Tuple2;
import org.apache.flink.api.java.tuple.Tuple3;
import org.apache.flink.api.java.utils.ParameterTool;
import org.apache.flink.configuration.Configuration;
import org.apache.flink.core.fs.FileSystem;

import java.io.File;
import java.io.Serializable;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.stream.Collectors;

public class CellCluster {

	public static class Tower implements Serializable {

		public double x, y;

		public int operator;

		public String radio;

		public Tower() {}

		public Tower(double x, double y) {
			this.x = x;
			this.y = y;
		}

		public Tower add(Tower other) {
			x += other.x;
			y += other.y;
			return this;
		}

		public Tower div(long val) {
			x /= val;
			y /= val;
			return this;
		}

		double euclideanDistance(Tower other) {
			return Math.sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
		}

		public void clear() {
			x = y = 0.0;
		}

		@Override
		public String toString() {
			return x + "," + y;
		}
	}

	public static class Centroid extends Tower {

		public int id;

		public Centroid() {}

		public Centroid(int id, double x, double y) {
			super(x, y);
			this.id = id;
		}

		public Centroid(int id, Tower p) {
			super(p.x, p.y);
			this.id = id;
		}

		@Override
		public String toString() {
			return id + "," + super.toString();
		}
	}

	private static String input;
	private static List<Integer> mobileOperators = new ArrayList<>();
	private static long numberOfClusters = -1;

	public static void main(String[] args) throws Exception {
		final ParameterTool params = ParameterTool.fromArgs(args);

		final ExecutionEnvironment env = ExecutionEnvironment.getExecutionEnvironment();

		env.getConfig().setGlobalJobParameters(params);

		if (!params.has("input")) {
			System.err.println("Use --input to specify valid file input.");
			System.exit(-1);
		}

		File inputFile = new File(params.get("input"));
		if (!inputFile.exists() || inputFile.isDirectory()) {
			System.err.println("Provided input file does not exist.");
			System.exit(-1);
		}

		input = params.get("input");

		if (!params.has("iterations")) {
			System.out.println("Use --iterations to specify a number of iterations for k-Means. Using 10 as default.");
		}

		int iterations = Integer.parseInt(params.get("iterations", "10"));

		if (params.has("mnc")) {
			mobileOperators = Arrays.stream(params.get("mnc").split(","))
					.map(Integer::parseInt).collect(Collectors.toList());
		}

		if (!params.has("k")) {
			System.out.println("No value for number of clusters (k) provided. Will fallback to number of LTE towers.");
		}
		numberOfClusters = Integer.parseInt(params.get("k", "-1"));

		if (!params.has("output")) {
			System.err.println("Use --output to specify valid file output.");
			System.exit(-1);
		}

		DataSet<Tower> points = getPointDataSet(env);
		DataSet<Centroid> centroids = getCentroidDataSet(env);

		IterativeDataSet<Centroid> loop = centroids.iterate(iterations);

		DataSet<Centroid> newCentroids = points
				.map(new SelectNearestCenter()).withBroadcastSet(loop, "centroids")
				.map(new CountAppender())
				.groupBy(0).reduce(new CentroidAccumulator())
				.map(new CentroidAverager());

		DataSet<Centroid> finalCentroids = loop.closeWith(newCentroids);

		DataSet<Tuple2<Integer, Tower>> clusteredPoints = points
				.map(new SelectNearestCenter()).withBroadcastSet(finalCentroids, "centroids");

		clusteredPoints.writeAsCsv(params.get("output"), "\n", ",", FileSystem.WriteMode.OVERWRITE)
				.setParallelism(1);
		env.execute("Exercise 4 - Part 2 - CellCluster on cellular towers");
	}

	private static DataSet<Centroid> getCentroidDataSet(ExecutionEnvironment env) throws Exception {
		DataSet<Centroid> centroids;

		centroids = env.readCsvFile(input)
				.ignoreFirstLine()
				.fieldDelimiter(",")
				.includeFields(true, false, true, false, true, false, true, true, false, false, false, false, false, false)
				.pojoType(Centroid.class, "radio", "operator", "id", "x", "y")
				.filter(centroid -> centroid.radio.equalsIgnoreCase("LTE"))
				.filter(centroid -> (mobileOperators.size() < 1 || mobileOperators.contains(centroid.operator)));

		if (numberOfClusters > 0) {
			centroids = centroids.first((int) numberOfClusters);
		} else {
			numberOfClusters = centroids.count();
		}

		return centroids;
	}

	private static DataSet<Tower> getPointDataSet(ExecutionEnvironment env) {
		DataSet<Tower> towers;
		towers = env.readCsvFile(input)
				.ignoreFirstLine()
				.fieldDelimiter(",")
				.includeFields(true, false, true, false, false, false, true, true, false, false, false, false, false, false)
				.pojoType(Tower.class, "radio", "operator", "x", "y")
				.filter(tower -> !tower.radio.equalsIgnoreCase("LTE"))
				.filter(tower -> (mobileOperators.size() < 1 || mobileOperators.contains(tower.operator)));

		return towers;
	}

	@FunctionAnnotation.ForwardedFields("*->1")
	public static final class SelectNearestCenter extends RichMapFunction<Tower, Tuple2<Integer, Tower>> {
		private Collection<Centroid> centroids;

		@Override
		public void open(Configuration parameters) {
			this.centroids = getRuntimeContext().getBroadcastVariable("centroids");
		}

		@Override
		public Tuple2<Integer, Tower> map(Tower p) {

			double minDistance = Double.MAX_VALUE;
			int closestCentroidId = -1;

			for (Centroid centroid : centroids) {
				double distance = p.euclideanDistance(centroid);

				if (distance < minDistance) {
					minDistance = distance;
					closestCentroidId = centroid.id;
				}
			}

			return new Tuple2<>(closestCentroidId, p);
		}
	}

	@FunctionAnnotation.ForwardedFields("f0;f1")
	public static final class CountAppender implements MapFunction<Tuple2<Integer, Tower>, Tuple3<Integer, Tower, Long>> {

		@Override
		public Tuple3<Integer, Tower, Long> map(Tuple2<Integer, Tower> t) {
			return new Tuple3<>(t.f0, t.f1, 1L);
		}
	}

	@FunctionAnnotation.ForwardedFields("0")
	public static final class CentroidAccumulator implements ReduceFunction<Tuple3<Integer, Tower, Long>> {

		@Override
		public Tuple3<Integer, Tower, Long> reduce(Tuple3<Integer, Tower, Long> val1, Tuple3<Integer, Tower, Long> val2) {
			return new Tuple3<>(val1.f0, val1.f1.add(val2.f1), val1.f2 + val2.f2);
		}
	}

	@FunctionAnnotation.ForwardedFields("0->id")
	public static final class CentroidAverager implements MapFunction<Tuple3<Integer, Tower, Long>, Centroid> {

		@Override
		public Centroid map(Tuple3<Integer, Tower, Long> value) {
			return new Centroid(value.f0, value.f1.div(value.f2));
		}
	}
}
